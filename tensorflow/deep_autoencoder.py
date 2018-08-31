"""Pre-training a deep autoencoder for MNIST dataset.

See http://www.cs.toronto.edu/~hinton/MatlabForSciencePaper.html.
"""

import load_mnist as mnist
import rbm_new as rbm


def _main():
    data, _ = mnist.MNIST("train",
                          path="../../machine-learning/data/mnist/",
                          data_size=40, batch_size=8,
                          reshape=False, one_hot=False, binarize=True).to_ndarray()

    max_epoch = 10

    # Layer 1
    print("----- Layer 1 -----")
    layer_i = rbm.RBM(train_data=data, num_hidden=1000)
    layer_i.train(max_epoch=max_epoch)
    # layer_i_param = (layer_i.weight, layer_i.visible_bias, layer_i.hidden_bias)

    # Layer 2
    print("----- Layer 2 -----")
    layer_ii = rbm.RBM(train_data=layer_i.hidden_data, num_hidden=500)
    layer_ii.train(max_epoch=max_epoch)
    # layer_ii_param = (layer_ii.weight, layer_ii.visible_bias, layer_ii.hidden_bias)

    # Layer 3
    print("----- Layer 3 -----")
    layer_iii = rbm.RBM(train_data=layer_ii.hidden_data, num_hidden=250)
    layer_iii.train(max_epoch=max_epoch)
    # layer_iii_param = (layer_iii.weight, layer_iii.visible_bias, layer_iii.hidden_bias)

    # Layer 4
    print("----- Layer 4 -----")
    layer_iv = rbm.RBM(train_data=layer_iii.hidden_data, num_hidden=30)
    layer_iv.train(max_epoch=max_epoch)
    # layer_iv_param = (layer_iv.weight, layer_iv.visible_bias, layer_iv.hidden_bias)


if __name__ == "__main__":
    _main()
