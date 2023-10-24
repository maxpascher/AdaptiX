from mlpluginapi import MLPluginAPI
import unreal_engine as ue

import os
import json
import tensorflow as tf
tf.config.experimental.set_memory_growth((tf.config.list_physical_devices('GPU'))[0], True)

# Global variables
IMG_HEIGHT, IMG_WIDTH = 240, 424 # Shape of the images
segments_used = (7, 27) # Index list of segments used.

def list2direction(theList):
    """Converts a list to a direction object"""
    return {'location': { 'x':    theList[0], 'y':    theList[1], 'z':    theList[2]},
            'rotation': { 'roll': theList[3], 'pitch':theList[4], 'yaw':  theList[5]},
            'gripper':  theList[6] }
def array2AdaptiveSet(array):
    """Converts an array to an AdaptiveSet Object consisting of direction objects"""
    return { f'direction_{i}': list2direction(a) for i,a in enumerate(array)}

#MLPluginAPI
class AdaptiveAPI(MLPluginAPI):

    def loadCNN(self, path):
        """Loads the CNN and prepares everything for prediction"""
        self.model = tf.keras.models.load_model(path, custom_objects = {s: lambda a,b:0 for s in \
                                            ['mahalanobis', 'log_likelyhood', 'weightedMetric']})
        ue.log('loadCNN finished')
        return "success"

    @tf.function
    def predictData(self, D, S):
        # preprocess unsorted data to datapoint
        #RGB = tf.reshape(RGB, (IMG_HEIGHT, IMG_WIDTH,3))
        D = tf.reshape(D, (IMG_HEIGHT, IMG_WIDTH,1))
        S = tf.reshape(S, (IMG_HEIGHT, IMG_WIDTH))

        #RGB = tf.image.convert_image_dtype(png, tf.float32, name="RGB_RangeChange")

        D = tf.cast(D*10, tf.uint16, name="Depth_float16_cm_to_uint16_mm")  # UE4 to realsense
        D = tf.cast(D, tf.float32, name="Depth_uint16_to_float32")
        D = tf.math.reciprocal_no_nan(D, name="Invert_Depth")


        S_l = [S==i for i in segments_used] # List of existing segments
        S0 = tf.zeros((IMG_HEIGHT, IMG_WIDTH), tf.bool)
        S = tf.stack([S0,]+S_l, axis=-1, name="Segments_one_hot")
        S = tf.cast(S, tf.float32, name="Segments_bool_to_float")
        S.set_shape((IMG_HEIGHT, IMG_WIDTH, len(segments_used)+1))

        datapoint = tf.expand_dims(tf.concat([S, D], axis=2, name="Stack_S_D"), axis=0)

        #predict
        cov = self.model(datapoint)[0]

        # postprocess covariance to eigenvalues
        eigVals, eigVects = tf.linalg.eigh(cov)
        eigVects = tf.reverse(eigVects, [-1]) # each column is an eigenvector
        eigVects_listed = tf.transpose(eigVects) # Transpose such that each row is an eigenvector

        #tf.py_function(self.storeIntermediate, [cov, eigVals], [])
        
        return eigVects_listed

    # Input in JSON form. Output will be auto converted to json
    def calculateCNN(self, theData):
        #self.storeInput(theData)
        dataObject = json.loads(theData)
        #RGB = tf.convert_to_tensor(dataObject['rgb'], dtype=tf.uint8) # do not forget to convert RGB-range if used!
        D =   tf.convert_to_tensor(dataObject['depth'], dtype=tf.float16)
        S =   tf.convert_to_tensor(dataObject['segments'], dtype=tf.uint8)

        eigVects = self.predictData(D,S).numpy().tolist()
        output = array2AdaptiveSet(eigVects)
        #self.storeOutput(output)
        return output

    def storeIntermediate(self, cov, eigVals):
        self.cov = cov.numpy()
        self.eigVals = tf.reverse(eigVals, [-1]).numpy()

    def storeInput(self, theData):
        path = "C:/Users/Felix/Desktop/enableme-unreal-project/Adaptive/inputs"
        path = os.path.join(path, f"{len(os.listdir(path))}.json")
        with open(path, 'w') as f:
            f.write(theData)
    def storeOutput(self, output):
        path = "C:/Users/Felix/Desktop/enableme-unreal-project/Adaptive/inputs"
        path = os.path.join(path, f"{len(os.listdir(path))}_out.json")
        with open(path, 'w') as f:
            json.dump(output, f, indent=4)        

#NOTE: this is a module function, not a class function. Change your CLASSNAME to reflect your class
#required function to get our api
def get_api():
    return AdaptiveAPI.get_instance()
