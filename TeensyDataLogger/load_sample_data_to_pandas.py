import numpy as np 
import pandas
import os

def get_filenames(path, contains, does_not_contain=['~', '.pyc']):
    cmd = 'ls ' + '"' + path + '"'
    ls = os.popen(cmd).read()
    all_filelist = ls.split('\n')
    try:
        all_filelist.remove('')
    except:
        pass
    filelist = []
    for i, filename in enumerate(all_filelist):
        if contains in filename:
            fileok = True
            for nc in does_not_contain:
                if nc in filename:
                    fileok = False
            if fileok:
                filelist.append( os.path.join(path, filename) )
    return filelist

def load_buffers_from_file(filename, data_type, number_fill_bytes, number_data_records=10):
    # nested datatype
    dt = np.dtype([('head', '<u4'), ('buff', data_type, (number_data_records,)), ('tail', 'i1', number_fill_bytes)])

    # read the data
    data = np.fromfile(filename, dtype=dt)
    correct_number_data_records = data['head'][0] # the correct number

    if correct_number_data_records != number_data_records:
        dt = np.dtype([('head', '<u4'), ('buff', data_type, (correct_number_data_records,)), ('tail', 'i1', number_fill_bytes)])
        data = np.fromfile(filename, dtype=dt)

    return np.hstack( data['buff'] )

def get_number_data_records(filename, data_type):
    dt = np.dtype([('head', '<u4'), ('buff', data_type, (5,)), ('tail', 'i1', 1)])
    data = np.fromfile(filename, dtype=dt)
    correct_number_data_records = data['head'][0] # the correct number
    return correct_number_data_records

def load_data_from_directory(data_directory, data_type, number_fill_bytes): 
    filenames = get_filenames(data_directory, '.bin')

    number_data_records = get_number_data_records(filenames[0], data_type)

    df = None
    for filename in filenames:
        data_buff = load_buffers_from_file(filename, data_type, number_fill_bytes, number_data_records)
        if df is None:
            df = pandas.DataFrame(data_buff)
        else:
            df = pandas.concat([df, pandas.DataFrame(data_buff)], ignore_index=True)

    return df

if __name__ == '__main__':

    # where your binary data files are
    data_directory = 'sample_data'

    # data column names and corresponding types based on the struct_t in the teensy firmware
    # see: https://numpy.org/devdocs/user/basics.types.html
    data_type = [('time', '<u4'),  # uint32_t time;
                 ('test1', '<u4'), # uint32_t test1;
                 ('test2', 'S24')] # char test2[24];

    # how many fill bytes are there? from teensy code.
    # would be nice if this could be determined automatically...
    # probably can be done given size of the file, size of the header, size of the buffer from the datatype
    number_fill_bytes = 28

    dataframe = load_data_from_directory(data_directory, data_type, number_fill_bytes)






