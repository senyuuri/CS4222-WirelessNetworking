from sklearn.naive_bayes import BernoulliNB
from sklearn.externals import joblib
import numpy

# training
# ----------------------
data = numpy.loadtxt(open("training.csv", "rb"), delimiter=",", skiprows=1)
bnb = BernoulliNB()
bnb.fit(data[:, 4:8], data[:, 8])
# dump trained model
joblib.dump(bnb, 'bnb_model.pkl') 


# prediction
# -----------------------
# load Bernoulli Naive Bayes model
# bnb = joblib.load('bnb_model.pkl') 

# t_data = numpy.loadtxt(open("walk_3/testing.csv", "rb"), delimiter=",")
# prediction = bnb.predict(t_data[:, 4:8])

# with open('walk_3/output.csv', 'w') as fout:
#     for i in range(t_data.shape[0]):
#         line = str(t_data[i][0]) + ',' + str(prediction[i]) + '\n'
#         fout.write(line)