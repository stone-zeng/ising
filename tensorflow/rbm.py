import os

import imageio
import matplotlib.pyplot as plt
import numpy as np
import tensorflow as tf

import load_mnist as mnist

# See http://lyy1994.github.io/machine-learning/2017/04/17/RBM-tensorflow-implementation.html

def weight(shape, name='weights'):
    return tf.Variable(tf.truncated_normal(shape, stddev=0.1), name=name)
def bias(shape, name='biases'):
    return tf.Variable(tf.constant(0.1, shape=shape), name=name)

class RBM:
    i = 0 # fliping index for computing pseudo likelihood

    def __init__(self, n_visible=784, n_hidden=500, k=30, momentum=False):
        self.n_visible = n_visible
        self.n_hidden = n_hidden
        self.k = k

        self.lr = tf.placeholder(tf.float32)
        if momentum:
            self.momentum = tf.placeholder(tf.float32)
        else:
            self.momentum = 0.0
        self.w = weight([n_visible, n_hidden], 'w')
        self.hb = bias([n_hidden], 'hb')
        self.vb = bias([n_visible], 'vb')

        self.w_v = tf.Variable(tf.zeros([n_visible, n_hidden]), dtype=tf.float32)
        self.hb_v = tf.Variable(tf.zeros([n_hidden]), dtype=tf.float32)
        self.vb_v = tf.Variable(tf.zeros([n_visible]), dtype=tf.float32)

    def propup(self, visible):
        pre_sigmoid_activation = tf.matmul(visible, self.w) + self.hb
        return tf.nn.sigmoid(pre_sigmoid_activation)

    def propdown(self, hidden):
        pre_sigmoid_activation = tf.matmul(hidden, tf.transpose(self.w)) + self.vb
        return tf.nn.sigmoid(pre_sigmoid_activation)

    def sample_h_given_v(self, v_sample):
        h_props = self.propup(v_sample)
        h_sample = tf.nn.relu(tf.sign(h_props - tf.random_uniform(tf.shape(h_props))))
        return h_sample

    def sample_v_given_h(self, h_sample):
        v_props = self.propdown(h_sample)
        v_sample = tf.nn.relu(tf.sign(v_props - tf.random_uniform(tf.shape(v_props))))
        return v_sample

    def CD_k(self, visibles):
        # k steps gibbs sampling
        v_samples = visibles
        h_samples = self.sample_h_given_v(v_samples)
        for _ in range(self.k):
            v_samples = self.sample_v_given_h(h_samples)
            h_samples = self.sample_h_given_v(v_samples)

        h0_props = self.propup(visibles)
        w_positive_grad = tf.matmul(tf.transpose(visibles), h0_props)
        w_negative_grad = tf.matmul(tf.transpose(v_samples), h_samples)
        w_grad = (w_positive_grad - w_negative_grad) / tf.to_float(tf.shape(visibles)[0])
        hb_grad = tf.reduce_mean(h0_props - h_samples, 0)
        vb_grad = tf.reduce_mean(visibles - v_samples, 0)
        return w_grad, hb_grad, vb_grad

    def learn(self, visibles):
        w_grad, hb_grad, vb_grad = self.CD_k(visibles)
        # compute new velocities
        new_w_v = self.momentum * self.w_v + self.lr * w_grad
        new_hb_v = self.momentum * self.hb_v + self.lr * hb_grad
        new_vb_v = self.momentum * self.vb_v + self.lr * vb_grad
        # update parameters
        update_w = tf.assign(self.w, self.w + new_w_v)
        update_hb = tf.assign(self.hb, self.hb + new_hb_v)
        update_vb = tf.assign(self.vb, self.vb + new_vb_v)
        # update velocities
        update_w_v = tf.assign(self.w_v, new_w_v)
        update_hb_v = tf.assign(self.hb_v, new_hb_v)
        update_vb_v = tf.assign(self.vb_v, new_vb_v)

        return [update_w, update_hb, update_vb, update_w_v, update_hb_v, update_vb_v]

    def sampler(self, visibles, steps=5000):
        v_samples = visibles
        for _ in range(steps):
            v_samples = self.sample_v_given_h(self.sample_h_given_v(v_samples))
        return v_samples

    def free_energy(self, visibles):
        first_term = tf.matmul(visibles, tf.reshape(self.vb, [tf.shape(self.vb)[0], 1]))
        second_term = tf.reduce_sum(
            tf.log(1 + tf.exp(self.hb + tf.matmul(visibles, self.w))), axis=1)
        return - first_term - second_term

    def pseudo_likelihood(self, visibles):
        x = tf.round(visibles)
        x_fe = self.free_energy(x)
        split0, split1, split2 = tf.split(x, [self.i, 1, tf.shape(x)[1] - self.i - 1], 1)
        xi = tf.concat([split0, 1 - split1, split2], 1)
        self.i = (self.i + 1) % self.n_visible
        xi_fe = self.free_energy(xi)
        return tf.reduce_mean(self.n_visible * tf.log(tf.nn.sigmoid(xi_fe - x_fe)), axis=0)


def save_images(images, size, path):
    """
    Save the samples images.
    The best size number is
            int(max(sqrt(image.shape[0]),sqrt(image.shape[1]))) + 1
    example:
        The batch_size is 64, then the size is recommended [8, 8]
        The batch_size is 32, then the size is recommended [6, 6]
    """
    img = (images + 1.0) / 2.0
    h, w = img.shape[1], img.shape[2]
    merge_img = np.zeros((h * size[0], w * size[1]))
    for idx, image in enumerate(images):
        i = idx % size[1]
        j = idx // size[1]
        merge_img[(j * h) : (j * h + h), (i * w) : (i * w + w)] = image
    return imageio.imwrite(path, merge_img)


def train(train_data, epoches):
    logs_dir = "./logs"
    samples_dir = "./samples"

    x = tf.placeholder(tf.float32, shape=[None, 784])
    noise_x, _ = train_data.sample_batch()
    # noise_x = tf.random_normal([train_data.batch_size, 784])
    rbm = RBM()
    step = rbm.learn(x)
    sampler = rbm.sampler(x, 200) # TODO
    pl = rbm.pseudo_likelihood(x)

    saver = tf.train.Saver()
    print("Initialization finished!")

    with tf.Session() as sess:
        init = tf.global_variables_initializer()
        sess.run(init)
        mean_cost = []
        epoch = 1
        for i in range(epoches * train_data.batch_num):
            # Draw samples (10 images)
            if i % (epoches * train_data.batch_num / 10) == 0:
                samples = sess.run(sampler, feed_dict = {x: noise_x})
                samples = samples.reshape([train_data.batch_size, 28, 28])
                save_images(samples, [8, 8],
                            os.path.join(samples_dir,
                                         'iteration_%d.png' % (i / train_data.batch_num)))
                print('Saved samples.')
            batch_x, _ = train_data.next_batch()
            sess.run(step, feed_dict = {x: batch_x, rbm.lr: 0.1})
            cost = sess.run(pl, feed_dict = {x: batch_x})
            mean_cost.append(cost)
            # Save model
            if i is not 0 and train_data.batch_index is 0:
                checkpoint_path = os.path.join(logs_dir, 'model.ckpt')
                saver.save(sess, checkpoint_path, global_step = epoch + 1)
                print('Saved Model.')
            # Print pseudo likelihood
            if i is not 0 and train_data.batch_index is 0:
                print('Epoch %d Cost %g' % (epoch, np.mean(mean_cost)))
                mean_cost = []
                epoch += 1
        print('Test')
        samples = sess.run(sampler, feed_dict = {x: noise_x})
        samples = samples.reshape([train_data.batch_size, 28, 28])
        save_images(samples, [8, 8], os.path.join(samples_dir, 'test.png'))
        print('Saved samples.')


data_path = "../../machine-learning/data/mnist/"
train_data = mnist.MNIST("train", data_path, data_size=256, batch_size=64)
# test_data = mnist.MNIST("test", data_path)

train(train_data, 10)
