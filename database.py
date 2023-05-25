import datetime
import sqlite3


"""
In this file, all database actions take place
The database of choice is sqlite, which is 
a built in python database library

conn: serves a connection to the database "data.db", takes two extra parameters
which are set to work better with timestamps, more on that can be found in the link below
https://pynative.com/python-sqlite-date-and-datetime/

cursor: is the object which executes all the queries send to the database

Those two objects are initialized multiple times in the file, this is due to me not being
able to make the program run smoothly with only one instantiation, I tried static ones,
global ones, but everytime I tried to reduce the number of inits, the program wouldn't wor properly
"""
conn = sqlite3.connect('data.db', detect_types=sqlite3.PARSE_DECLTYPES | sqlite3.PARSE_COLNAMES)
cursor = conn.cursor()


if __name__ == '__main__':
   conn = sqlite3.connect('data.db', detect_types=sqlite3.PARSE_DECLTYPES | sqlite3.PARSE_COLNAMES)
   cursor = conn.cursor()
  #
   data = ("test", 100, datetime.datetime.now().replace(microsecond=0))
   #cursor.execute(f"INSERT INTO userscores VALUES (?, ?, ?)", data)
   cursor.execute("DELETE FROM userscores WHERE username = Test")
   entries = cursor.fetchall()
   for entry in entries:
       print(entry)


def insert_data(data):
    """
    This method takes incoming data as a parameter and saves it into the database
    All the data comes combined in one single dictionary

    The three lists split up the data, so they can be written easy to the database

    :var: namelist is wrongfully named a lists, when in reality, it is a dictionary
    containing the aforementioned three lists with the sensor data
    its purpose is to make storing data in the database more conveninet and readable


    :param data: is a dictionary containing (usually 60 sets of) data in addition to
    a timestamp, which will also be saved into the database
    """
    def format_data(data_part, i, name):
        return data_part[f'{name}_x'][i], data_part[f'{name}_y'][i], data_part[f'{name}_z'][i], data_part["time"][i]

    conn = sqlite3.connect('data.db', detect_types=sqlite3.PARSE_DECLTYPES | sqlite3.PARSE_COLNAMES)
    cursor = conn.cursor()

    tuplelistbeschl = [format_data(data, entry, "beschl") for entry in range(len(data['beschl_x']))]
    tuplelistgyro = [format_data(data, entry, "gyro") for entry in range(len(data['gyro_x']))]
    tuplelistmag = [format_data(data, entry, "mag") for entry in range(len(data['mag_x']))]

    namelist = dict(beschleunigung=tuplelistbeschl, gyroscop=tuplelistgyro, magnetometer=tuplelistmag)
    for name, data in namelist.items():
        cursor.executemany(f"INSERT INTO {name} VALUES (?, ?, ?, ?)", data)
        cursor.execute(f"SELECT rowid, * FROM {name} ")
        entries = cursor.fetchall()
        for entry in entries:
            print(f" rowID: {entry[0]} {name}_x:{entry[1]}   {name}_y:{entry[2]}   {name}_z:{entry[3]} {entry[4]}")


def insert_quater(data):
    conn = sqlite3.connect('data.db', detect_types=sqlite3.PARSE_DECLTYPES | sqlite3.PARSE_COLNAMES)
    cursor = conn.cursor()

    def format_data(data_part, i):
        return data_part["q0"][i], data_part["q1"][i], data_part["q2"][i], data_part["q3"][i], data_part["time"][i]

    tuplelistquater = [format_data(data, entry) for entry in range(len(data['q1']))]
    cursor.executemany(f"INSERT INTO quaterionen VALUES (?, ?, ?, ?, ?)", tuplelistquater)


def insert_user(name, dt=datetime.datetime.now().replace(microsecond=0)):
    """
    stores username in database, if it isn't there already
    :param name: name to save in database
    :param dt: timestamp, defaulting to current time, to determine user creation time
    """
    conn = sqlite3.connect('data.db', detect_types=sqlite3.PARSE_DECLTYPES | sqlite3.PARSE_COLNAMES)
    cursor = conn.cursor()
    cursor.execute(f"SELECT * FROM userscores")
    namelist = cursor.fetchall()
    listname, _, _2 = zip(*namelist)
    if name not in listname:
        cursor.execute("INSERT INTO userscores VALUES (?, ?, ?)", (name, 0, dt))
    conn.commit()
    conn.close()


def insert_highscore(name, highscore, dt=datetime.datetime.now().replace(microsecond=0)):
    # Saves username along with the achieved highscore to database, again with a timestamp
    conn = sqlite3.connect('data.db', detect_types=sqlite3.PARSE_DECLTYPES | sqlite3.PARSE_COLNAMES)
    cursor = conn.cursor()
    cursor.execute("INSERT INTO userscores VALUES (?, ?, ?)", (name, highscore, dt))
    conn.commit()
    conn.close()


def get_usernames():
    """
    :return: all usernames currently saved in the database
    """
    conn = sqlite3.connect('data.db')
    cursor = conn.cursor()
    cursor.execute("SELECT username FROM userscores")
    entries = cursor.fetchall()
    return {entry for entry in entries}


def get_highscore(name):
    """
    :param name: username to receive highscore from
    :return: highscore of given user
    """
    conn = sqlite3.connect('data.db', detect_types=sqlite3.PARSE_DECLTYPES | sqlite3.PARSE_COLNAMES)
    cursor = conn.cursor()
    cursor.execute(f"SELECT * FROM userscores WHERE username = '{name}'")
    return max(cursor.fetchall())[1]


conn.commit()
conn.close()
