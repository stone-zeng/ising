"""
Backpropagation.
"""

import numpy as np
import utils


def backpropagation(layers: list, train_data, test_data, max_epoch: int):
    w_list = _initialize_weight(layers)
    # for i in w_list:
    #     print(i.shape)

    train_error = []
    test_error = []
    for epoch in range(max_epoch):
        train_error.append(_calculate_reconstruction_error(train_data, w_list))
        test_error.append(_calculate_reconstruction_error(test_data, w_list))

    print(train_error)
    print(test_error)


def _initialize_weight(layers: list):
    w_list = []
    for i in layers:
        w_list.append(np.concatenate((i.weight, i.hidden_bias[None, :])))
    for i in reversed(layers):
        w_list.append(np.concatenate((i.weight.transpose(), i.visible_bias[None, :])))
    return w_list


def _calculate_reconstruction_error(data, w_list: list):
    num_batches, batch_size, num_visible = data.shape

    num_layers = len(w_list) // 2
    error = 0.0

    for batch in range(num_batches):
        w_prob_list = [_right_extend(data[batch])]
        for i in range(num_layers - 1):
            w_prob_list.append(_right_extend(
                utils.sigmoid(np.matmul(w_prob_list[i], w_list[i]))))
        w_prob_list.append(_right_extend(
            np.matmul(w_prob_list[num_layers - 1], w_list[num_layers - 1])))
        for i in range(num_layers, 2 * num_layers):
            w_prob_list.append(_right_extend(
                utils.sigmoid(np.matmul(w_prob_list[i], w_list[i]))))

        data_in = w_prob_list[0][:, :-1]
        data_out = w_prob_list[-1][:, :-1]
        error += np.sum(np.square(data_in - data_out)) / batch_size

    return error


def _right_extend(array):
    """Add an column of value one to the right of `array`.
    """
    ones = np.ones((array.shape[0], 1), dtype=array.dtype)
    return np.concatenate((array, ones), axis=1)


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
    # _calculate_reconstruction_error(images)


if __name__ == "__main__":
    import load_mnist as mnist

    # Local MNIST data
    MNIST_PATH = "../../machine-learning/data/mnist/"
    _test()
