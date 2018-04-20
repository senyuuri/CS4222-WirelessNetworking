from sklearn.naive_bayes import BernoulliNB
from sklearn.externals import joblib
import numpy

data = numpy.loadtxt(open("training.csv", "rb"), delimiter=",")
bnb = BernoulliNB()
bnb.fit(data[:, 0], data[:, 1])


# load Bernoulli Naive Bayes model
# bnb = joblib.load('bnb_model.pkl') 

# dump trained model
# joblib.dump(bnb, 'bnb_model.pkl') 