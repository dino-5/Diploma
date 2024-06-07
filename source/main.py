import tensorflow as tf
from tensorflow.keras.layers import Conv2D, Input, UpSampling2D
from tensorflow.keras.models import Model
import numpy as np
import os
from PIL import Image
import matplotlib.pyplot as plt
import tensorflow_datasets as tfds

def getTrainIterator():
    return trainDataset.as_numpy_iterator()

def getValidationIterator():
    return validationDataset.as_numpy_iterator()

div2kImageData = tfds.load('div2k')
trainDataset= div2kImageData['train']
validationDataset= div2kImageData['validation']

popularResolutions = dict()
for i in getTrainIterator():
    shape = i['lr'].shape
    if shape in popularResolutions.keys():
      popularResolutions[shape]+=1
    else:
      popularResolutions[shape]=1

result = sorted(popularResolutions.items(), key = lambda element : element[1]) 
mostPopularResolution = result[-1][0]


def getElementsWithMostPopularRes(iterator):
  outDataset = []
  for element in iterator:
    if element['lr'].shape == mostPopularResolution:
      outDataset.append(element['lr'])
  return outDataset

trainDatasetFiltered = getElementsWithMostPopularRes(getTrainIterator()) 
validationDatasetFiltered = getElementsWithMostPopularRes(getValidationIterator()) 
imageArray = []

def res(res, factor):
   return (res[0]//factor, res[1]//factor)

def saveImages(dataset, type):
  for index in range(len(dataset)):
    lowRes = np.array(dataset[index])
    endShape = (lowRes.shape[1], lowRes.shape[0])

    image= Image.fromarray(lowRes, mode="RGB").resize(res(endShape, 4), Image.BICUBIC)
    image.save(f"../data/tensorflow-datasets/{type}/high/{index}.jpg")

    image= image.resize(res(endShape, 8), Image.BICUBIC)
    image= image.resize(res(endShape, 4), Image.BICUBIC)
    image.save(f"../data/tensorflow-datasets/{type}/low/{index}.jpg")

saveImages(trainDatasetFiltered, 'train')
saveImages(validationDatasetFiltered, 'validation')
   