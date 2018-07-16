"""Training Restricted Boltzmann Machine (RBM).

See http://www.cs.toronto.edu/~hinton/MatlabForSciencePaper.html.
"""

import tensorflow as tf


class RBM:
    """Training Restricted Boltzmann Machine (RBM).

    Arguments are directly from the original MATLAB code:

    Constants
    ---------
    + `_num_visible`  -> `numdims`
    + `_num_hidden`   -> `numhid`
    + `_num_epochs`  -> `maxepoch`
    + `_num_batches`  -> `numbatches`
    + `_batch_size`   -> `numcases`

    Variables
    ---------
    + `weight`            -> `vishid`
    + `visible_bias`      -> `hidbiases`
    + `hidden_bias`       -> `visbiases`
    + `_weight_inc`       -> `vishidinc`
    + `_vb_inc` -> `hidbiasinc`
    + `_hb_inc`  -> `visbiasinc`
    """

    def __init__(self,
                 num_visible: int,
                 num_hidden: int,
                 num_batches: int,
                 batch_size: int):
        """Construct RBM.

        Args:
            num_visible: Number of visible units.
            num_hidden: Number of hidden units.
            num_batches: Number of mini-batches in the dataset.
            batch_size: Size of a mini-batch.
        """
        self._num_visible = num_visible
        self._num_hidden = num_hidden
        self._num_batches = num_batches
        self._batch_size = batch_size

        self._learning_rate = tf.placeholder(dtype=tf.float32, name="learning_rate")
        self._momentum = tf.placeholder(dtype=tf.float32, name="momentum")
        self._weight_cost = tf.placeholder(dtype=tf.float32, name="weigh_cost")

        # Initialize symmetric weights and biases
        self.w = tf.Variable(
            0.1 * tf.random_normal(shape=[self._num_visible, self._num_hidden]), name="weight")
        self.vb = self._zeros_variable(shape=[1, self._num_visible], name="visible_bias")
        self.hb = self._zeros_variable(shape=[1, self._num_hidden], name="hidden_bias")

        # Initialize increments (velocities) of weights and biases
        self._w_inc = self._zeros_variable(name="weight_inc")
        self._vb_inc = self._zeros_variable(shape=[1, self._num_visible], name="visible_bias_inc")
        self._hb_inc = self._zeros_variable(shape=[1, self._num_hidden], name="hidden_bias_inc")

        self._error_sum = tf.Variable(0.0, name="error_sum")

    def _zeros_variable(self, shape=None, name: str = None):
        """Create a Variable with all elements set to 0.

        Args:
            shape: Default value is the same as the shape of weights.
            name: A name for the operation (optional).
        """
        if shape is None:
            shape = [self._num_visible, self._num_hidden]
        return tf.Variable(tf.zeros(shape=shape), name=name)

    def update_weights_and_biases(self, batch_data):
        """Update weights, bias and their increments (velocities), as well as error.

        Args:
            batch_data: Data in a mini-batch. Its shape should be `[batch_size, num_visible]`.

        Return:
            A (2D) tuple of assignment operations for parameters and increments.
        """
        # Gradients
        (w_grad, vb_grad, hb_grad), error = self._gradient(batch_data)
        # Weight-decay (L2 regularization)
        w_decay = self._weight_cost * self.w
        # New increments (velocities)
        w_inc = self._momentum * self._w_inc + self._learning_rate * (w_grad - w_decay)
        vb_inc = self._momentum * self._vb_inc + self._learning_rate * vb_grad
        hb_inc = self._momentum * self._hb_inc + self._learning_rate * hb_grad
        # Assignment operations for parameters and increments
        update_params = (tf.assign(self.w, self.w + w_inc),
                         tf.assign(self.vb, self.vb + vb_inc),
                         tf.assign(self.hb, self.hb + hb_inc))
        update_incs = (tf.assign(self._w_inc, w_inc),
                       tf.assign(self._vb_inc, vb_inc),
                       tf.assign(self._hb_inc, hb_inc))
        update_error = tf.assign(self._error_sum, self._error_sum + error)
        return (update_params, update_incs), update_error

    def _gradient(self, batch_data):
        """Calculate gradients of weights and bias.

        Args:
            batch_data: Data in a mini-batch. Its shape should be `[batch_size, num_visible]`.

        Returns:
            A tuple of gradients of weights, bias for visible units and bias for hidden units.
        """
        pos_products, pos_v_act, pos_h_act, neg_data = self._pos_phase(batch_data)
        neg_products, neg_v_act, neg_h_act = self._neg_phase(neg_data)
        # Gradients
        w_grad = (pos_products - neg_products) / self._batch_size
        vb_grad = (pos_v_act - neg_v_act) / self._batch_size
        hb_grad = (pos_h_act - neg_h_act) / self._batch_size
        # Error
        error = tf.reduce_sum(tf.square(batch_data - neg_data))
        return (w_grad, vb_grad, hb_grad), error

    def _pos_phase(self, data):
        """Evaluate the positive phase.

        Args:
            data: Data to be fed into visible units. Its shape should be
                `[batch_size, num_visible]`.
        """
        # Hidden states, using probability itself. Shape = [_batch_size, _num_hidden]
        pos_h_probs = tf.sigmoid(
            tf.matmul(data, self.w) + tf.tile(self.hb, [self._batch_size, 1]))
        # TODO: batchposhidprobs(:,:,batch)=poshidprobs;
        pos_products = tf.matmul(data, pos_h_probs, transpose_a=True)
        # Shape = [1, _num_visible] / [1, _num_hidden]
        # act = activation
        # TODO: can be replaced by `tf.reduce_mean`
        pos_v_act = tf.reduce_sum(data, 0, keepdims=True)
        pos_h_act = tf.reduce_sum(pos_h_probs, 0, keepdims=True)
        pos_h_states = tf.to_float(
            pos_h_probs > tf.random_uniform([self._batch_size, self._num_hidden]))
        # Shape = [_batch_size, _num_visible]
        neg_data = tf.sigmoid(
            tf.matmul(pos_h_states, self.w, transpose_b=True)
            + tf.tile(self.vb, [self._batch_size, 1]))
        return pos_products, pos_v_act, pos_h_act, neg_data

    def _neg_phase(self, neg_data):
        """Evaluate the negative phase.

        Args:
            neg_data: shape = [`_batch_size`, `_num_hidden`]
        """
        # Hidden states, using probability itself. Shape = [_batch_size, _num_hidden]
        neg_h_probs = tf.sigmoid(
            tf.matmul(neg_data, self.w) + tf.tile(self.hb, [self._batch_size, 1]))
        neg_products = tf.matmul(neg_data, neg_h_probs, transpose_a=True)
        # Shape = [1, _num_visible] / [1, _num_hidden]
        # TODO: can be replaced by `tf.reduce_mean`
        neg_v_act = tf.reduce_sum(neg_data, 0, keepdims=True)
        neg_h_act = tf.reduce_sum(neg_h_probs, 0, keepdims=True)
        return neg_products, neg_v_act, neg_h_act

    def train(self, train_data,
              num_epochs: int,
              learning_rate: float = 0.1,
              momentum: float = None,
              weight_cost: float = 0.0002):
        """The main training procedure.

        Args:
            `train_data`: shape = [`_batch_size`, `_num_visible`, `_num_batches`]
        """
        x = tf.placeholder(tf.float32, shape=[self._batch_size, self._num_visible])
        step = self.update_weights_and_biases(x)

        error_list = []

        with tf.Session() as sess:
            sess.run(tf.global_variables_initializer())
            for epoch in range(num_epochs):
                print("Epoch:", epoch + 1)  # TODO
                if momentum is None:
                    if epoch > 5:
                        momentum = 0.9  # final value
                    else:
                        momentum = 0.5  # initial value
                for batch_index in range(self._num_batches):
                    batch_data = train_data[:, :, batch_index]
                    _, error = sess.run(step, feed_dict={x: batch_data,
                                                         self._learning_rate: learning_rate,
                                                         self._momentum: momentum,
                                                         self._weight_cost: weight_cost})
                    error_list.append(error)
                    print("\tBatch:", batch_index + 1, error)  # TODO


def _test():
    rbm = RBM(num_visible=4, num_hidden=3, num_batches=5, batch_size=10)
    data = np.random.normal(size=(10, 4, 5))
    rbm.train(data, num_epochs=20)


if __name__ == "__main__":
    import numpy as np
    _test()
