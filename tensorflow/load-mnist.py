import struct
import numpy as np
import matplotlib.pyplot as plt

data_path = "../../machine-learning/data/mnist/"

# test_image = open(data_path + "t10k-images-idx3-ubyte" , 'rb')
# buf = test_image.read()
# 
# image_index = struct.calcsize('>IIII')
# temp = struct.unpack_from('>784B', buf, image_index)
# im = np.reshape(temp, (28, 28))
# 
# plt.imshow(im , cmap='gray')

def readfile(name, path):
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

def get_image(buf):
    image_index = 0
    image_index += struct.calcsize('>IIII')
    im = []
    for _ in range(9): # `9` is a sample here
        temp = struct.unpack_from('>784B', buf, image_index)
        im.append(np.reshape(temp, (28, 28)))
        image_index += struct.calcsize('>784B')
    return im

image, label = readfile("test", data_path)

im = get_image(image)
plt.imshow(im[1], cmap='gray')

binarify = lambda x: 0 if x <= 127 else 1
binarify_v = np.vectorize(binarify)

im_bin = binarify_v(im[1])
plt.imshow(im_bin, cmap='gray')
plt.show()
