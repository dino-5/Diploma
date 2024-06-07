import tensorflow as tf
from tensorflow.keras.layers import Conv2D, Input, UpSampling2D
from tensorflow.keras.models import Model
import numpy as np
import os
from PIL import Image
import matplotlib.pyplot as plt
import tensorflow_datasets as tfds
import datetime 
import matplotlib.pyplot as plt
import argparse as cmd_args

parser = cmd_args.ArgumentParser(prog = 'Upscaler')
parser.add_argument('-f', '--first') 
parser.add_argument('-s', '--second') 
parser.add_argument('-e', '--epochs') 
parser.add_argument('-b', '--batch') 
parser.add_argument('-i', '--image') 

args = parser.parse_args()
first = args.first
second = args.second
epoch = args.epochs
batch = args.batch
image = args.image

def upsample_image(model, low_res_image):
    low_res_image = np.expand_dims(low_res_image, axis=0)  # Додавання виміру партії
    sr_image = model.predict(low_res_image)
    sr_image = np.squeeze(sr_image, axis=0)  # Видалення виміру партії
    sr_image = np.clip(sr_image, 0, 1)  # Обрізка значень до [0, 1]
    return sr_image

type = "train"
imageDir = f"../data/tensorflow-datasets/{type}"

model = tf.keras.models.load_model(f'model/weights_{first}_{first}_{second}_{epoch}_{batch}.keras')
print("Saved models")

example_low_res_image = np.array(Image.open(f"{imageDir}/low/{image}.jpg")) / 255.0
high_res_image = np.array(Image.open(f"{imageDir}/high/{image}.jpg")) / 255.0
example_sr_image = upsample_image(model, example_low_res_image)
print("Upsampled image")

plt.figure(figsize=(15, 5))

plt.subplot(1, 3, 1)
plt.title('Низька розмірність')
plt.imshow(example_low_res_image)

plt.subplot(1, 3, 2)
plt.title('Результат роботи моделі')
plt.imshow(example_sr_image)

plt.subplot(1, 3, 3)
plt.title('Початкове зображення')
plt.imshow(high_res_image)

plt.show()



