from pymongo import MongoClient
# pprint library is used to make the output look more pretty
from prettyprinter import pprint
# connect to MongoDB, change the << MONGODB URL >> to reflect your own connection string
client = MongoClient('mongodb+srv://user:cqXSE7EzLgh2LC9@beta.d501j.mongodb.net/spot?retryWrites=true&w=majority')
db=client.db
# Issue the serverStatus command and print the results
# serverStatusResult=db.command("serverStatus")
# pprint(serverStatusResult)

sensordatas=db.sensordatas

pprint(sensordatas)
