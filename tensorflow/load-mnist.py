import struct
import numpy as np

def read_file(name, path):
    """Read `name` in `path`. `name` can be either `test` or `train`."""
    if name == "test":
        image_file_name = path + "t10k-images-idx3-ubyte"
        label_file_name = path + "t10k-labels-idx1-ubyte"
    if name == "train":
        image_file_name = path + "train-images-idx3-ubyte"
        label_file_name = path + "train-labels-idx1-ubyte"
    with open(image_file_name,'rb') as image_file:
        image_buf = image_file.read()
    with open(label_file_name,'rb') as label_file:
        label_buf = label_file.read()
    return image_buf, label_buf

def get_image(img_buf):
    """Get an image array from `img_buf`. MNIST images have a size of 28 by 28."""
    img_buf_len = len(img_buf)
    img_buf_idx = struct.calcsize('>IIII') # Skip the first 16 number
    img_arr = []
    while img_buf_idx < img_buf_len:
        temp = struct.unpack_from('>784B', img_buf, img_buf_idx)
        img_arr.append(np.reshape(temp, (28, 28)))
        img_buf_idx += struct.calcsize('>784B')
    return img_arr

def get_label(lbl_buf):
    """Get an label array from `lbl_buf`."""
    lbl_buf_len = len(lbl_buf)
    lbl_buf_idx = struct.calcsize('>II') # Skip the first 8 number
    lbl_arr = []
    while lbl_buf_idx < lbl_buf_len:
        lbl_arr.append(struct.unpack_from('>B', lbl_buf, lbl_buf_idx)[0])
        lbl_buf_idx += struct.calcsize('>B')
    return lbl_arr

def image_binarify(img):
    """Binarify an image or an image array."""
    binarify_vectorized = np.vectorize(lambda x: 0 if x <= 127 else 1)
    return binarify_vectorized(img)

def main(idx):
    data_path = "../../machine-learning/data/mnist/"
    image_buffer, label_buffer = read_file("test", data_path)
    image = get_image(image_buffer)
    plt.imshow(image[idx], cmap='gray')
    plt.show()
    image = image_binarify(image)
    plt.imshow(image[idx], cmap='gray')
    plt.show()
    label = get_label(label_buffer)
    print(label[idx])

if __name__ == '__main__':
    import sys
    import matplotlib.pyplot as plt
    if len(sys.argv) > 1:
        index = int(sys.argv[1])
    else:
        index = 0
    main(index)
