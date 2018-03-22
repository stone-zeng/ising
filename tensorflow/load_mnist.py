import struct
import numpy as np

class MNIST:
    """
    Loading MNIST dataset.

    Args:
      data_type: Can be either `test` or `train`.
      path: Path for MNIST data.
      batch_size: Size of mini-batch. Default value `None` means using the whole dataset as a batch.
      binarify: Whether to binarify the images (using 0 and 1 values).
      reshape: Whether to reshape the images into 2D arrays.
      one_hot: whether to use one-hot encoding for labels.
      random_seed: Random seed to shuffle data.
    """
    batch_index = 0

    def __init__(self, data_type, path,
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
        _image_buf, _label_buf = self.read_file()
        if binarify:
            self.images = self.image_binarify(self.get_image(_image_buf))
        else:
            self.images = self.get_image(_image_buf)
        self.labels = self.get_label(_label_buf)
        # Size
        self.size = len(self.images)
        if batch_size == None:
            self.batch_size = self.size
        else:
            self.batch_size = batch_size

    def read_file(self):
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

    def get_image(self, image_buf):
        """Get an image array from `image_buf`. MNIST images have a size of 28 by 28."""
        image_buf_len = len(image_buf)
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

    def get_label(self, label_buf):
        """Get an label array from `label_buf`."""
        label_buf_len = len(label_buf)
        label_buf_idx = struct.calcsize(">II") # Skip the first 8 number
        lable_arr = []
        while label_buf_idx < label_buf_len:
            lable_arr.append(struct.unpack_from(">B", label_buf, label_buf_idx)[0])
            label_buf_idx += struct.calcsize(">B")
        return lable_arr

    def image_binarify(self, image):
        """Binarify an image or an image array."""
        binarify_vectorized = np.vectorize(lambda x: 0 if x <= 127 else 1)
        return binarify_vectorized(image)

def main(idx):
    data_path = "../../machine-learning/data/mnist/"
    data = MNIST("test", data_path, reshape=True)
    print("Dataset size:", data.size)
    print("Batch size:", data.batch_size)
    plt.imshow(data.images[idx], cmap="gray")
    plt.show()
    print("Number:", data.labels[idx])

if __name__ == "__main__":
    import sys
    import matplotlib.pyplot as plt
    if len(sys.argv) > 1:
        idx = int(sys.argv[1])
    else:
        idx = 0
    main(idx)
