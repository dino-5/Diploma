import tensorflow as tf
from tensorflow.keras.layers import Conv2D, Input, UpSampling2D
from tensorflow.keras.models import Model
import numpy as np
import os
from PIL import Image
import matplotlib.pyplot as plt
import tensorflow_datasets as tfds
import datetime 

import argparse as cmd_args

parser = cmd_args.ArgumentParser(prog = 'Upscaler')
parser.add_argument('-f', '--first') 
parser.add_argument('-s', '--second') 
parser.add_argument('-e', '--epochs') 
parser.add_argument('-b', '--batch') 

args = parser.parse_args()


dataDir = "../data/tensorflow-datasets/"
print("Num GPUs Available: ", len(tf.config.list_physical_devices('GPU')))


def load_image(folder, type='train', res='low'):
    folderFullName = f"{folder}/{type}/{res}/"
    imageArray = []
    for fileName in os.listdir(folderFullName):
        imageArray.append(np.array(Image.open(os.path.join(folderFullName, fileName)))/255.0)
    print("end")
    return np.array(imageArray)
    
first_layer = int(args.first) # кількість фільтрів в першому шарі мережі
second_layer = int(args.second) # кількість фільтрів в другому шарі мережі

def create_model(shape):
    inputs = Input(shape)

    x = Conv2D(first_layer, (9, 9), activation='relu', padding='same')(inputs)
    
    # Шар нелінійного відображення
    x = Conv2D(second_layer, (1, 1), activation='relu', padding='same')(x)
    
    # Шар реконструкції
    outputs = Conv2D(3, (5, 5), activation='linear', padding='same')(x)
    
    model = Model(inputs, outputs)
    return model  

model_config = f"{first_layer}_{first_layer}_{second_layer}"

Train = 'train'
Validation = 'validation'
lowRes = 'low'
highRes = 'high'
print("program started", datetime.datetime.now())

print("start loading")
lowResTrainImage = load_image(dataDir, Train, lowRes)
print("loaded low res train image")
highResTrainImage = load_image(dataDir, Train, highRes)
print("loaded high res train image")

#lowResValidationImage = load_image(dataDir, Validation, lowRes)
#highResValidationImage = load_image(dataDir, Validation, highRes)
print("loaded images")

print(lowResTrainImage[0].shape)
model = create_model(lowResTrainImage[0].shape)
model.compile(optimizer='adam', loss='mean_squared_error')
print(model.summary())
print("created model")

# Навчання моделі
print("learning started", datetime.datetime.now())
# Setup TensorBoard callback
log_dir = "logs/fit/" + datetime.datetime.now().strftime("%Y%m%d-%H%M%S")
tensorboard_callback = tf.keras.callbacks.TensorBoard(log_dir=log_dir, histogram_freq=1)
# Train your model with the TensorBoard callback
# To start TensorBoard after training
# tensorboard --logdir=./logs

epochs = int(args.epochs)
batch_size = int(args.batch)
history = model.fit(x=lowResTrainImage, y=highResTrainImage, epochs=epochs, batch_size=batch_size, 
                    validation_split=0.1, callbacks=[tensorboard_callback])

print(history)
print("learning ended", datetime.datetime.now())
print("trained model")
learning_config = f"{epochs}_{batch_size}"
model.save(f"model/weights_{model_config}_{learning_config}.keras")

plt.plot(history.history['accuracy'])
plt.plot(history.history['val_accuracy'])
plt.title('Model accuracy')
plt.ylabel('Accuracy')
plt.xlabel('Epoch')
plt.legend(['Train', 'Val'], loc='upper left')
plt.show()


print("program ended", datetime.datetime.now())


