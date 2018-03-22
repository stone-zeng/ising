import random
import struct

import numpy as np

class MNIST:
    """
    Loading MNIST dataset.

    Args:
      data_type: Can be either `test` or `train`.
      path: Path for MNIST data.
      data_size: Size of dataset. Default value `None` means using all data in MNIST.
      batch_size: Size of mini-batch. Default value `None` means using the whole dataset as a batch.
      binarify: Whether to binarify the images (using 0 and 1 values).
      reshape: Whether to reshape the images into 2D arrays.
      one_hot: whether to use one-hot encoding for labels.
      random_seed: Random seed to shuffle data.
    """
    batch_index = 0

    def __init__(self, data_type, path,
                 data_size=None,
                 batch_size=None,
                 binarify=True,
                 reshape=False,
                 one_hot=False,
                 random_seed=0):
        self.data_type = data_type
        self.path = path
        # Options
        self.binarify = binarify
        self.reshape = reshape
        self.one_hot = one_hot
        self.random_seed = random_seed
        # Data
        _image_buf, _label_buf = self._read_file()
        if binarify:
            self._images = self._image_binarify(self._get_image(_image_buf, data_size))
        else:
            self._images = self._get_image(_image_buf, data_size)
        self._labels = self._get_label(_label_buf, data_size)
        # Size
        self.data_size = int(len(self._images))
        if batch_size == None:
            self.batch_size = self.data_size
        else:
            self.batch_size = batch_size
        self.batch_num = int(self.data_size / self.batch_size)

    def _read_file(self):
        if self.data_type == "test":
            image_file_name = self.path + "t10k-images-idx3-ubyte"
            label_file_name = self.path + "t10k-labels-idx1-ubyte"
        if self.data_type == "train":
            image_file_name = self.path + "train-images-idx3-ubyte"
            label_file_name = self.path + "train-labels-idx1-ubyte"
        with open(image_file_name,"rb") as image_file:
            image_buf = image_file.read()
        with open(label_file_name,"rb") as label_file:
            label_buf = label_file.read()
        return image_buf, label_buf

    def _get_image(self, image_buf, image_num):
        """Get an image array from `image_buf`. MNIST images have a size of 28 by 28."""
        if image_num == None:
            image_buf_len = len(image_buf)
        else:
            image_buf_len = image_num * 784
        image_buf_idx = struct.calcsize(">IIII") # Skip the first 16 number
        image_arr = []
        while image_buf_idx < image_buf_len:
            temp = struct.unpack_from(">784B", image_buf, image_buf_idx)
            if self.reshape:
                image_arr.append(np.reshape(temp, (28, 28)))
            else:
                image_arr.append(temp)
            image_buf_idx += struct.calcsize(">784B")
        return image_arr

    def _get_label(self, label_buf, label_num):
        """Get an label array from `label_buf`."""
        if label_num == None:
            label_buf_len = len(label_buf)
        else:
            label_buf_len = label_num * 8
        label_buf_idx = struct.calcsize(">II") # Skip the first 8 number
        lable_arr = []
        while label_buf_idx < label_buf_len:
            temp = struct.unpack_from(">B", label_buf, label_buf_idx)[0]
            if self.one_hot:
                v = np.zeros(10)
                v[temp] = 1.0
                lable_arr.append(v)
            else:
                lable_arr.append(temp)
            label_buf_idx += struct.calcsize(">B")
        return lable_arr

    def _image_binarify(self, image):
        """Binarify an image or an image array."""
        binarify_vectorized = np.vectorize(lambda x: 0 if x <= 127 else 1)
        return binarify_vectorized(image)

    def next_batch(self):
        begin = self.batch_index * self.batch_size
        end = (self.batch_index + 1) * self.batch_size
        self.batch_index = (self.batch_index + 1) % self.batch_num
        return self._images[begin:end], self._labels[begin:end]

    def sample_batch(self):
        index = random.randrange(self.batch_num)
        begin = index * self.batch_size
        end = (index + 1) * self.batch_size
        return self._images[begin:end], self._labels[begin:end]

def main(index):
    data_path = "../../machine-learning/data/mnist/"
    data = MNIST("test", data_path, reshape=True, data_size=5, one_hot=True)
    print("Dataset size:", data.data_size)
    print("Batch size:", data.batch_size)
    plt.imshow(data._images[index], cmap="gray")
    plt.show()
    print("Number:", data._labels[index])

if __name__ == "__main__":
    import sys
    import matplotlib.pyplot as plt
    if len(sys.argv) > 1:
        index = int(sys.argv[1])
    else:
        index = 0
    main(index)
