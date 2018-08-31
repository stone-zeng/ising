import numpy as np
import tensorflow as tf
import matplotlib.pyplot as plt

x_train = np.array(list(range(1, 50 + 1))).astype(np.float64)
y_train = np.array(\
    [
        40.05, 11.53, 21.25, 49.13, 16.40,
        26.83, 24.74, 41.21, 38.73, 29.52,
        45.63, 54.34, 50.27, 51.84, 33.57,
        40.11, 40.44, 42.70, 44.05, 51.41,
        45.76, 48.39, 54.61, 50.69, 52.74,
        77.78, 66.67, 72.80, 81.29, 72.19,
        65.75, 84.58, 90.20, 92.31, 74.87,
        94.30, 91.45, 85.14, 79.91, 86.07,
        93.03, 88.04, 88.79, 97.40, 86.34,
        98.82, 93.41, 80.03, 96.17, 99.33
    ])
print("Training data:\n",
      "X:", len(x_train), "\tType:", x_train.dtype, "\n",
      "Y:", len(y_train), "\tType:", y_train.dtype, "\n",
      "\n")

model_dir        = ".\\temp1"
model_dir_custom = ".\\temp2"

# Linear regression estimator
feature_columns = [tf.feature_column.numeric_column("x", shape=[1])]
estimator = tf.estimator.LinearRegressor(feature_columns=feature_columns,
                                         model_dir=model_dir)

input_fn = tf.estimator.inputs.numpy_input_fn(
    {"x": x_train}, y_train, num_epochs=None, shuffle=True)
train_input_fn = tf.estimator.inputs.numpy_input_fn(
    {"x": x_train}, y_train, shuffle=True)

# Train the model
estimator.train(input_fn=input_fn, steps=100)

# Evaluate the model on the training set
train_metrics = estimator.evaluate(input_fn=train_input_fn)
print("Train metrics: %r"% train_metrics)

# Show training result
# The names can be found by `estimator.get_variable_names()`
weight_train = estimator.get_variable_value("linear/linear_model/x/weights")[0, 0]
bias_train   = estimator.get_variable_value("linear/linear_model/bias_weights")[0]
print("Model parameters:\n",
      "Weight:", weight_train, "\n",
      "Bias:  ", bias_train)

# Custom estimator
def model_fn(features, labels, mode):
    W = tf.get_variable("W", [1], dtype=tf.float64)
    b = tf.get_variable("b", [1], dtype=tf.float64)
    y = W * features["x"] + b
    # Loss function
    loss = tf.reduce_sum(tf.square(y - labels))
    # Train function
    learning_rate = 0.00001
    global_step = tf.train.get_global_step()
    optimizer = tf.train.GradientDescentOptimizer(learning_rate)
    train = tf.group(optimizer.minimize(loss),
    tf.assign_add(global_step, 1))
    return tf.estimator.EstimatorSpec(
        mode=mode,
        predictions=y,
        loss=loss,
        train_op=train)

estimator_custom = tf.estimator.Estimator(model_fn=model_fn,
                                          model_dir=model_dir_custom)

input_fn_custom = tf.estimator.inputs.numpy_input_fn(
    {"x": x_train}, y_train, batch_size=4, num_epochs=None, shuffle=True)
train_input_fn_custom = tf.estimator.inputs.numpy_input_fn(
    {"x": x_train}, y_train, batch_size=4, num_epochs=1000, shuffle=True)

# Train the model
estimator_custom.train(input_fn=input_fn_custom, steps=100)

train_metrics_custom = estimator.evaluate(input_fn=train_input_fn_custom)
print("Train metrics: %r"% train_metrics_custom)

weight_train_custom = estimator_custom.get_variable_value("W")[0]
bias_train_custom   = estimator_custom.get_variable_value("b")[0]
print("Model parameters:\n",
      "Weight:", weight_train_custom, "\n",
      "Bias:  ", bias_train_custom)

# Show data and fit curve
fit = x_train * weight_train + bias_train
fit_custom = x_train * weight_train_custom + bias_train_custom
plt.plot(x_train, y_train, "ro", label="Original data")
plt.plot(fit,        "b", label="Fit curve")
plt.plot(fit_custom, "g", label="Fit curve (custom)")
plt.legend()
plt.show()
