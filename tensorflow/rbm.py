"""Training Restricted Boltzmann Machine (RBM).

See http://www.cs.toronto.edu/~hinton/MatlabForSciencePaper.html.
"""

# import tensorflow as tf
import numpy as np
import utils


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

    def __init__(
            self,
            train_data,
            num_hidden: int,
            learning_rate=0.1,
            weight_cost=0.0002,
            initial_momentum=0.5,
            final_momentum=0.9,
            momentum_epoch_threshold=5):
        self._data = train_data
        self._num_batches, self._batch_size, self._num_visible = train_data.shape
        self._num_hidden = num_hidden
        self._learning_rate = learning_rate
        self._weight_cost = weight_cost
        self._momentum = initial_momentum
        self._final_momentum = final_momentum
        self._momentum_epoch_threshold = momentum_epoch_threshold
        self._initialize_variables()

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
        print(self._data.shape)
        for epoch in range(max_epoch):
            print("Epoch:", epoch + 1)
            if epoch > self._momentum_epoch_threshold:
                self._momentum = self._final_momentum

            self.hidden_data = []
            for batch in range(self._num_batches):
                print("\tEpoch:", epoch + 1, "\tBatch:", batch + 1)

                neg_data = self._positive_phase(self._data[batch])
                self._negative_phase(neg_data)
                self._check_error()
                self._update()

            self.hidden_data = np.array(self.hidden_data)

    def _positive_phase(self, data):
        """Evaluate the positive phase.

        Args:
            data: Data in a mini-batch that is to be fed into visible units. Its shape should be
                `(batch_size, num_visible)`.
        """
        # Hidden states, i.e. the probability that hidden units equal to 1.
        # Shape = `(batch_size, num_hidden)`
        pos_hidden_probs = utils.sigmoid(np.matmul(data, self.weight) +
                                         np.tile(self.hidden_bias, (self._batch_size, 1)))
        self.hidden_data.append(pos_hidden_probs)
        self._pos_products = np.matmul(data.transpose(), pos_hidden_probs)
        # Positive
        # Shape = [1, _num_visible] / [1, _num_hidden]
        # act = activation
        self._pos_visible_act = np.mean(data, axis=0)
        self._pos_hidden_act = np.mean(pos_hidden_probs, axis=0)
        pos_hidden_states = (pos_hidden_probs >
                             np.random.rand(self._batch_size, self._num_hidden)).astype(float)
        # Shape = [_batch_size, _num_visible]
        neg_data = utils.sigmoid(np.matmul(pos_hidden_states, self.weight.transpose()) +
                                 np.tile(self.visible_bias, (self._batch_size, 1)))
        return neg_data

    def _negative_phase(self, neg_data):
        """Evaluate the negative phase.

        Args:
            neg_data: shape = [`_batch_size`, `_num_hidden`]
        """
        # Hidden states, using probability itself. Shape = [_batch_size, _num_hidden]
        neg_h_probs = utils.sigmoid(np.matmul(neg_data, self.weight) +
                                    np.tile(self.hidden_bias, (self._batch_size, 1)))
        self._neg_products = np.matmul(neg_data.transpose(), neg_h_probs)
        # Shape = [1, _num_visible] / [1, _num_hidden]
        self._neg_visible_act = np.mean(neg_data, axis=0)
        self._neg_hidden_act = np.mean(neg_h_probs, axis=0)

    def _check_error(self):
        pass

    def _update(self):
        # Update increments (gradients)
        self._w_inc = self._momentum * self._w_inc + \
            self._learning_rate * ((self._pos_products - self._neg_products) / self._batch_size -
                                   self._weight_cost * self.weight)
        self._vb_inc = self._momentum * self._vb_inc + \
            self._learning_rate * (self._pos_visible_act - self._neg_visible_act)
        self._hb_inc = self._momentum * self._hb_inc + \
            self._learning_rate * (self._pos_hidden_act - self._neg_hidden_act)
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
    print("num_hidden:", num_hidden, "\n")
    model = RBM(train_data=images, num_hidden=num_hidden)
    model.train(max_epoch=5)


if __name__ == "__main__":
    import load_mnist as mnist

    # Local MNIST data
    MNIST_PATH = "../../machine-learning/data/mnist/"
    _test()
