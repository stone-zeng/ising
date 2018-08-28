"""Loading MNIST dataset.
"""

import struct
import numpy as np


class MNIST:
    """
    Loading MNIST dataset.

    In the directory of MNIST dataset, there should be the following files:
        - Training set:
           - train-images-idx3-ubyte
           - train-labels-idx1-ubyte
        - Test set:
           - t10k-images-idx3-ubyte
           - t10k-labels-idx1-ubyte

    Functions
    ---------

        next_batch()
        image_pair(index: int)
        sample_batch(batch_index: int)
        to_ndarray()

    Attributes
    ----------

        data_type: Can be either `"test"` or `"train"`.
        path: Path for MNIST data.
        data_size: Size of the dataset. Default value `None` means using all data in MNIST.
        batch_size: Size of the mini-batch. Default value `None` means using the whole dataset as
            a mini-batch.
        binarize: Whether to binarize the images (using 0 and 1 values). Default value is True.
        reshape: Whether to reshape the images into 2D arrays. Default value is False.
        one_hot: whether to use one-hot encoding for labels (e.g. using vector
            `[1, 0, 0, 0, 0, 0, 0, 0, 0, 0]` for 0). Default value is False.
    """

    IMAGE_SIZE = 784
    LABEL_SIZE = 1
    _IMAGE_SIZE_FMT = ">784B"
    _LABEL_SIZE_FMT = ">B"
    IMAGE_SHAPE = (28, 28)

    batch_index = 0

    def __init__(self, data_type: str, path: str,
                 data_size: int = None,
                 batch_size: int = None,
                 binarize=True,
                 reshape=False,
                 one_hot=False):
        self.data_type = data_type
        self.path = path
        # Options
        self.binarize = binarize
        self.reshape = reshape
        self.one_hot = one_hot
        # Data buffer
        # `data_size` will be updated according to the actual data
        image_buf, label_buf = self._read_file()
        # Size
        if data_size is None:
            # `len(image_buf)` may not be exactly divided by 784
            self.data_size = len(image_buf) // self.IMAGE_SIZE
        else:
            self.data_size = data_size
        if batch_size is None:
            self.batch_size = self.data_size
        else:
            if batch_size < self.data_size:
                self.batch_size = batch_size
            else:
                raise ValueError("batch size larger than data size")
        self.batch_num = self.data_size // self.batch_size
        # Data
        self._images = self._get_image(image_buf)
        self._labels = self._get_label(label_buf)

    def _read_file(self):
        if self.data_type == "test":
            image_file_name = self.path + "t10k-images-idx3-ubyte"
            label_file_name = self.path + "t10k-labels-idx1-ubyte"
        elif self.data_type == "train":
            image_file_name = self.path + "train-images-idx3-ubyte"
            label_file_name = self.path + "train-labels-idx1-ubyte"
        else:
            raise ValueError("only type \"test\" and \"train\" are available")
        # "rb" means reading + binary mode
        with open(image_file_name, "rb") as image_file:
            image_buf = image_file.read()
        with open(label_file_name, "rb") as label_file:
            label_buf = label_file.read()
        return image_buf, label_buf

    def _get_image(self, image_buf):
        """Get an image array from `image_buf`.

        This is the structure of the image file (training set):

            [offset] [type]          [value]          [description]
            0000     32 bit integer  0x00000803(2051) magic number
            0004     32 bit integer  60000            number of images
            0008     32 bit integer  28               number of rows
            0012     32 bit integer  28               number of columns
            0016     unsigned byte   ??               pixel
            0017     unsigned byte   ??               pixel
            ........
            xxxx     unsigned byte   ??               pixel
        """
        image_buf_len = self.data_size * self.IMAGE_SIZE + 16
        image_offset = 16
        image_arr = []
        while image_offset < image_buf_len:
            temp = struct.unpack_from(self._IMAGE_SIZE_FMT, image_buf, image_offset)
            if self.binarize:
                temp = np.vectorize(lambda x: 0 if x <= 127 else 1)(temp)
            if self.reshape:
                temp = np.reshape(temp, self.IMAGE_SHAPE)
            image_arr.append(temp)
            image_offset += self.IMAGE_SIZE
        return image_arr

    def _get_label(self, label_buf):
        """Get an label array from `label_buf`.

        This is the structure of the label file (training set):

            [offset] [type]          [value]          [description]
            0000     32 bit integer  0x00000801(2049) magic number (MSB first)
            0004     32 bit integer  60000            number of items
            0008     unsigned byte   ??               label
            0009     unsigned byte   ??               label
            ........
            xxxx     unsigned byte   ??               label
        """
        label_buf_len = self.data_size * self.LABEL_SIZE + 8
        label_offset = 8
        label_arr = []
        while label_offset < label_buf_len:
            temp = struct.unpack_from(self._LABEL_SIZE_FMT, label_buf, label_offset)[0]
            if self.one_hot:
                vec = np.zeros(10)
                vec[temp] = 1
                label_arr.append(vec)
            else:
                label_arr.append(temp)
            label_offset += self.LABEL_SIZE
        return label_arr

    def next_batch(self):
        """Increase `batch_index` by 1, then return a mini-batch of (image, label) tuples."""
        this_batch = self.batch(self.batch_index)
        self.batch_index = (self.batch_index + 1) % self.batch_num
        return this_batch

    def image_pair(self, index: int):
        """Return a (image, label) tuple at `index`."""
        if index < self.data_size:
            return self._images[index], self._labels[index]
        raise IndexError("image index out of range")

    def batch(self, batch_index: int):
        """Return a mini-batch of (image, label) tuples at `batch_index`."""
        if batch_index < self.batch_num:
            begin = batch_index * self.batch_size
            end = (batch_index + 1) * self.batch_size
            return self._images[begin:end], self._labels[begin:end]
        raise IndexError("batch index out of range")

    def to_ndarray(self):
        """Return the raw data tuple `(images, labels)` as `np.ndarray`.
        """
        images = []
        labels = []
        for i in range(self.batch_num):
            image, label = self.batch(i)
            images.append(image)
            labels.append(label)
        return np.asarray(images), np.asarray(labels)


def _test():
    data = MNIST("train", MNIST_PATH,
                 data_size=200, batch_size=8,
                 reshape=True, one_hot=False, binarize=False)
    print("Meta-data:")
    print("\tDataset size:", data.data_size)
    print("\tBatch size:", data.batch_size)

    col_num = 4
    row_num = data.batch_size // col_num + 1

    _test_random_images(data, col_num, row_num)
    _test_random_batch(data, col_num, row_num)
    _test_next_batch(data, col_num, row_num)


def _test_random_images(data, col_num, row_num):
    images = []
    labels = []
    for _ in range(10):
        index = random.randrange(data.data_size)
        image, label = data.image_pair(index)
        images.append(image)
        labels.append(label)
    _plot(images, labels, col_num=col_num, row_num=row_num)


def _test_random_batch(data, col_num, row_num):
    index = random.randrange(data.batch_num)
    images, labels = data.batch(index)
    _plot(images, labels, col_num=col_num, row_num=row_num)


def _test_next_batch(data, col_num, row_num):
    for _ in range(3):
        images, labels = data.next_batch()
        _plot(images, labels, col_num=col_num, row_num=row_num)


def _plot(images, labels, col_num, row_num):
    for i, (image, label) in enumerate(zip(images, labels)):
        plt.subplot(row_num, col_num, i + 1)
        plt.imshow(image, cmap="gray")
        plt.axis('off')
        plt.title(str(label))
    plt.show()


def _test_numpy():
    images, labels = MNIST("train", MNIST_PATH,
                           data_size=200, batch_size=8,
                           reshape=False, one_hot=False, binarize=False).to_ndarray()
    print(images.shape)                      # shape = (num_batches, batch_size, num_visible)
    print(np.moveaxis(images, 0, -1).shape)  # shape = (batch_size, num_visible, num_batches)
    print(labels.shape)  # shape = (num_batches, batch_size)


if __name__ == "__main__":
    import random
    import matplotlib.pyplot as plt

    # Local MNIST data
    MNIST_PATH = "../machine-learning/data/mnist/"

    _test()
    _test_numpy()
