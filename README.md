# CrossConnection
This is a project that can detect any cross connection that may occur during a phone call. This will inform the user via a notification that another interfering signal is there in between the phone call.

This will record the signal patterns on real-time basis and compare them at all times and whenever it detects another signal not matching the pattern it will inform the user that interference has been detected. I have used sliding window technique to compare and match the signals. Moreover, I have used the Eigen library and libsndfile framework of C++ to do the calculations and record the audio and convert them to values between -1 and 1 to emulate the amplitude of the signal.
