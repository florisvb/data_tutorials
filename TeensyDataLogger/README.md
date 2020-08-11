# Teensy Data Logger and Analysis

In the `teensy_data_logger` directory is a simple example of a data logger, based on Michael Stetner's code here: https://github.com/MichaelStetner/TeensyDataLogger/blob/master/TeensyDataLogger.ino. I tested it on a teensy 3.5. 


If you run that code it will generate a directory with N binary data files. An example of the data is included in the `sample_data` directory.


To load the binary data into python requires jumping through some hoops. The python file `load_sample_data_to_pandas.py` provides an example of how this is done. Note that you will need to change the analysis code depending on the data type and buffer size of the data!