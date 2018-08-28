"""Training Restricted Boltzmann Machine (RBM).

See http://www.cs.toronto.edu/~hinton/MatlabForSciencePaper.html.
"""

# import tensorflow as tf
import numpy as np


# TODO: Put into utils
def _sigmoid(x):
    """Logistic sigmoid function.
    """
    return 1 / (1 + np.exp(-x))


class RBM:
    """Training Restricted Boltzmann Machine (RBM).

    Arguments are directly from the original MATLAB code:

    Constants:
        `_num_visible` -> `numdims`
        `_num_hidden`  -> `numhid`
        `_num_batches` -> `numbatches`
        `_batch_size`  -> `numcases`

    Variables:
        `weight`       -> `vishid`
        `visible_bias` -> `hidbiases`
        `hidden_bias`  -> `visbiases`
        `_w_inc`       -> `vishidinc`
        `_vb_inc`      -> `hidbiasinc`
        `_hb_inc`      -> `visbiasinc`
    """

    def __init__(self, data, num_hidden: int):
        self._data = data
        self._num_hidden = num_hidden
        self._initialize_constants()
        self._initialize_variables()

    def _initialize_constants(
            self,
            learning_rate=0.1,
            weight_cost=0.0002,
            initial_momentum=0.5,
            final_momentum=0.9,
            momentum_epoch_threshold=5):
        self._learning_rate = learning_rate
        self._weight_cost = weight_cost
        self._momentum = initial_momentum
        self._final_momentum = final_momentum
        self._momentum_epoch_threshold = momentum_epoch_threshold
        # Data dimensions
        self._num_batches, self._batch_size, self._num_visible = self._data.shape

    def _initialize_variables(self):
        # Initialize symmetric weights and biases
        self.weight = 0.1 * np.random.randn(self._num_visible, self._num_hidden)
        self.visible_bias = np.zeros(self._num_visible)
        self.hidden_bias = np.zeros(self._num_hidden)
        # Initialize increments (velocities) of weights and biases
        self._w_inc = np.zeros_like(self.weight)
        self._vb_inc = np.zeros_like(self.visible_bias)
        self._hb_inc = np.zeros_like(self.hidden_bias)

    def train(self, max_epoch: int):
        """The main training procedure.
        """
        for epoch in range(max_epoch):
            print("Epoch:", epoch + 1)
            if epoch > self._momentum_epoch_threshold:
                self._momentum = self._final_momentum
            for batch in range(self._num_batches):
                print("\tEpoch:", epoch + 1, "\tBatch:", batch + 1)

                neg_data = self._positive_phase(self._data[batch])
                self._negative_phase(neg_data)
                self._check_error()
                self._update()

                print(self.weight)
                print(self.visible_bias)
                print(self.hidden_bias)

    def _positive_phase(self, data):
        """Evaluate the positive phase.

        Args:
            data: Data in a mini-batch that is to be fed into visible units. Its shape should be
                `(batch_size, num_visible)`.
        """
        # Hidden states, i.e. the probability that hidden units equal to 1.
        # Shape = `(batch_size, num_hidden)`
        pos_h_probs = _sigmoid(np.matmul(data, self.weight) +
                               np.tile(self.hidden_bias, (self._batch_size, 1)))
        # TODO: batchposhidprobs(:,:,batch)=poshidprobs;
        self._pos_products = np.matmul(data.transpose(), pos_h_probs)
        # Positive
        # Shape = [1, _num_visible] / [1, _num_hidden]
        # act = activation
        # TODO: can be replaced by `np.mean`
        self._pos_v_act = np.sum(data, axis=0)
        self._pos_h_act = np.sum(pos_h_probs, axis=0)
        pos_h_states = (pos_h_probs >
                        np.random.rand(self._batch_size, self._num_hidden)).astype(float)
        # Shape = [_batch_size, _num_visible]
        neg_data = _sigmoid(np.matmul(pos_h_states, self.weight.transpose()) +
                            np.tile(self.visible_bias, (self._batch_size, 1)))
        return neg_data

    def _negative_phase(self, neg_data):
        """Evaluate the negative phase.

        Args:
            neg_data: shape = [`_batch_size`, `_num_hidden`]
        """
        # Hidden states, using probability itself. Shape = [_batch_size, _num_hidden]
        neg_h_probs = _sigmoid(np.matmul(neg_data, self.weight) +
                               np.tile(self.hidden_bias, (self._batch_size, 1)))
        self._neg_products = np.matmul(neg_data.transpose(), neg_h_probs)
        # Shape = [1, _num_visible] / [1, _num_hidden]
        # TODO: can be replaced by `np.mean`
        self._neg_v_act = np.sum(neg_data, axis=0)
        self._neg_h_act = np.sum(neg_h_probs, axis=0)

    def _check_error(self):
        pass

    def _update(self):
        # Update increments (gradients)
        self._w_inc = self._momentum * self._w_inc + \
            self._learning_rate * ((self._pos_products - self._neg_products) / self._batch_size -
                                   self._weight_cost * self.weight)
        self._vb_inc = self._momentum * self._vb_inc + \
            self._learning_rate / self._batch_size * (self._pos_v_act - self._neg_v_act)
        self._hb_inc = self._momentum * self._hb_inc + \
            self._learning_rate / self._batch_size * (self._pos_h_act - self._neg_h_act)
        # Update weights and biases
        self.weight += self._w_inc
        self.visible_bias += self._vb_inc
        self.hidden_bias += self._hb_inc


def _test():
    # Data
    images, _ = mnist.MNIST("train", MNIST_PATH,
                            data_size=40, batch_size=8,
                            reshape=False, one_hot=False, binarize=True).to_ndarray()
    num_batches, batch_size, num_visible = images.shape
    print("num_batches:", num_batches)
    print("batch_size:", batch_size)
    print("num_visible:", num_visible)

    # Model
    num_hidden = 40
    print("num_visible:", num_hidden, "\n")
    model = RBM(data=images, num_hidden=num_hidden)
    model.train(max_epoch=1)


if __name__ == "__main__":
    import load_mnist as mnist

    # Local MNIST data
    MNIST_PATH = "../../machine-learning/data/mnist/"
    _test()

#class RBM:
#    """Training Restricted Boltzmann Machine (RBM).
#
#    Arguments are directly from the original MATLAB code:
#
#    Constants
#    ---------
#    + `_num_visible`  -> `numdims`
#    + `_num_hidden`   -> `numhid`
#    + `_num_epochs`  -> `maxepoch`
#    + `_num_batches`  -> `numbatches`
#    + `_batch_size`   -> `numcases`
#
#    Variables
#    ---------
#    + `weight`            -> `vishid`
#    + `visible_bias`      -> `hidbiases`
#    + `hidden_bias`       -> `visbiases`
#    + `_weight_inc`       -> `vishidinc`
#    + `_vb_inc` -> `hidbiasinc`
#    + `_hb_inc`  -> `visbiasinc`
#    """
#
#    def __init__(self, data, num_hidden: int):
#                #  num_visible: int,
#                #  num_hidden: int,
#                #  num_batches: int,
#                #  batch_size: int):
#        """Construct RBM.
#
#        Args:
#            num_visible: Number of visible units.
#            num_hidden: Number of hidden units.
#            num_batches: Number of mini-batches in the dataset.
#            batch_size: Size of a mini-batch.
#        """
#        self._num_batches, self._batch_size, self._num_visible = data.shape
#        self._num_hidden = num_hidden
#
#        # self._learning_rate = 0.1
#        # self._momentum = 0.5
#        # self._weight_cost = 0.0002
#
#        # Initialize symmetric weights and biases
#        self.weight = 0.1 * np.random.randn(self._num_visible, self._num_hidden)
#        self.visible_bias = np.zeros(self._num_visible)
#        self.hidden_bias = np.zeros(self._num_hidden)
#
#        # Initialize increments (velocities) of weights and biases
#        self._w_inc = np.zeros_like(self.weight)
#        self._vb_inc = np.zeros_like(self.visible_bias)
#        self._hb_inc = np.zeros_like(self.hidden_bias)
#
#        self._error_sum = 0.0
#
#    def _pos_phase(self, data):
#        """Evaluate the positive phase.
#
#        Args:
#            data: Data in a mini-batch that is to be fed into visible units. Its shape should be
#                `(batch_size, num_visible)`.
#        """
#        # Hidden states, i.e. the probability that hidden units equal to 1.
#        # Shape = `(batch_size, num_hidden)`
#        pos_h_probs = _sigmoid(
#            np.matmul(data, self.weight) + np.tile(self.hidden_bias, (self._batch_size, 1)))
#        # TODO: batchposhidprobs(:,:,batch)=poshidprobs;
#        pos_products = np.matmul(data.transpose(), pos_h_probs)
#        # Positive
#        # Shape = [1, _num_visible] / [1, _num_hidden]
#        # act = activation
#        # TODO: can be replaced by `np.mean`
#        pos_v_act = np.sum(data, axis=0)
#        pos_h_act = np.sum(pos_h_probs, axis=0)
#        pos_h_states = (pos_h_probs > np.random.rand(self._batch_size,
#                                                     self._num_hidden)).astype(float)
#        # Shape = [_batch_size, _num_visible]
#        neg_data = _sigmoid(
#            np.matmul(pos_h_states, self.weight.transpose()) +
#            np.tile(self.visible_bias, (self._batch_size, 1)))
#        return pos_products, pos_v_act, pos_h_act, neg_data
#
#    def _neg_phase(self, neg_data):
#        """Evaluate the negative phase.
#
#        Args:
#            neg_data: shape = [`_batch_size`, `_num_hidden`]
#        """
#        # Hidden states, using probability itself. Shape = [_batch_size, _num_hidden]
#        neg_h_probs = _sigmoid(
#            np.matmul(neg_data, self.weight) + np.tile(self.hidden_bias, (self._batch_size, 1)))
#        neg_products = np.matmul(neg_data.transpose(), neg_h_probs)
#        # Shape = [1, _num_visible] / [1, _num_hidden]
#        # TODO: can be replaced by `np.mean`
#        neg_v_act = np.sum(neg_data, axis=0)
#        neg_h_act = np.sum(neg_h_probs, axis=0)
#        return neg_products, neg_v_act, neg_h_act
#
#    def train(self, train_data,
#                num_epochs: int,
#                learning_rate: float = 0.1,
#                momentum: float = None,
#                weight_cost: float = 0.0002):
#        """The main training procedure.
#
#        Args:
#            `train_data`: shape = [`_batch_size`, `_num_visible`, `_num_batches`]
#        """
#        x = tf.placeholder(tf.float32, shape=[self._batch_size, self._num_visible])
#        step = self.update_weights_and_biases(x)
#
#        error_list = []
#
#        with tf.Session() as sess:
#            sess.run(tf.global_variables_initializer())
#            for epoch in range(num_epochs):
#                print("Epoch:", epoch + 1)  # TODO
#                if momentum is None:
#                    if epoch > 5:
#                        momentum = 0.9  # final value
#                    else:
#                        momentum = 0.5  # initial value
#                for batch_index in range(self._num_batches):
#                    batch_data = train_data[:, :, batch_index]
#                    _, error = sess.run(step, feed_dict={x: batch_data,
#                                                            self._learning_rate: learning_rate,
#                                                            self._momentum: momentum,
#                                                            self._weight_cost: weight_cost})
#                    error_list.append(error)
#                    print("\tBatch:", batch_index + 1, error)  # TODO
#
#
#def _test():
#    rbm = RBM(num_visible=4, num_hidden=3, num_batches=5, batch_size=10)
#    data = np.random.normal(size=(10, 4, 5))
#    rbm.train(data, num_epochs=20)
#
