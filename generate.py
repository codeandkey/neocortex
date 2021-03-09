# This class contains the neural network and provides methods for evaluating and training.

import os
import numpy as np
import tensorflow as tf
import sys

sys.setrecursionlimit(10000)

from tensorflow import keras
from tensorflow.keras import layers

SQUARE_BITS = 85
RESIDUAL_CONV_FILTERS = 16
RESIDUAL_LAYERS = 16

path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'model')

if len(sys.argv) > 1:
    path = sys.argv[1]

if os.path.exists(path):
    print('Path {0} already exists, terminating'.format(path))
    sys.exit(1)

print('Generating model')
inputs = keras.Input((8, 8, SQUARE_BITS), name='input')
legal_mask = keras.Input((4096,), name='legal_mask')

# Add first convolutional layers

x = layers.Conv2D(SQUARE_BITS, 3, padding='same')(inputs)

x = layers.BatchNormalization()(x)
x = layers.ReLU()(x)

# Add residual layers

def make_residual(x):
    skip = x

    for _ in range(RESIDUAL_CONV_FILTERS):
        x = layers.Conv2D(SQUARE_BITS, 3, padding='same')(x)

    x = layers.BatchNormalization()(x)
    x = layers.ReLU()(x)

    for _ in range(RESIDUAL_CONV_FILTERS):
        x = layers.Conv2D(SQUARE_BITS, 3, padding='same')(x)

    x = layers.BatchNormalization()(x)
    x = layers.Add()([x, skip])
    x = layers.ReLU()(x)

    return x

for _ in range(RESIDUAL_LAYERS):
    x = make_residual(x)

# Add value head

value_head = layers.Conv2D(SQUARE_BITS, 1)(x)
value_head = layers.BatchNormalization()(value_head)
value_head = layers.ReLU()(value_head)
value_head = layers.Dense(256)(value_head)
value_head = layers.ReLU()(value_head)
value_head = layers.Dense(256)(value_head)
value_head = layers.Flatten()(value_head)
value_head = layers.Dense(1, activation='tanh', name='value')(value_head)

# Add policy head

policy_head = layers.Conv2D(SQUARE_BITS, 1)(x)
policy_head = layers.Conv2D(SQUARE_BITS, 1)(policy_head)
policy_head = layers.BatchNormalization()(policy_head)
policy_head = layers.ReLU()(policy_head)
policy_head = layers.Flatten()(policy_head)
policy_head = layers.Dense(4096)(policy_head)
policy_head = layers.Multiply()([policy_head, legal_mask])
policy_head = layers.Softmax(name='policy')(policy_head)

model = keras.Model(inputs=[inputs, legal_mask], outputs=[policy_head, value_head])

model.compile(optimizer='Adagrad', loss='mse')
model.save(path)

print('Wrote new model to {0}'.format(path))
print(model.summary())
