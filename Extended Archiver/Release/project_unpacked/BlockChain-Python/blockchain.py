# import libraries
import hashlib
import random
import string
import json
import binascii
# import numpy as np
# import pandas as pd
# import pylab as pl
import logging
import datetime
import collections



# following imports are required by PKI
import Crypto
import Crypto.Random
from Crypto.Hash import SHA
from Crypto.PublicKey import RSA
from Crypto.Signature import PKCS1_v1_5



class Client:
	def __init__(self):
		random = Crypto.Random.new().read
		self._private_key = RSA.generate(1024, random)
		self._public_key = self._private_key.publickey()
		self._signer = PKCS1_v1_5.new(self._private_key)

	@property
	def identity(self):
		return binascii.hexlify(self._public_key.exportKey(format='DER')).decode('ascii')

Dinesh = Client()
# print (Dinesh.identity)


class Transaction:
	def __init__(self, sender, recipient, value):
		self.sender = sender
		self.recipient = recipient
		self.value = value
		self.time = datetime.datetime.now()

	def to_dict(self):
		if self.sender == "Genesis":
			identity = "Genesis"
		else:
			identity = self.sender.identity

		return collections.OrderedDict({
			'sender': identity,
			'recipient': self.recipient,
			'value': self.value,
			'time' : self.time})
			
	def sign_transaction(self):
		private_key = self.sender._private_key
		signer = PKCS1_v1_5.new(private_key)
		h = SHA.new(str(self.to_dict()).encode('utf8'))
		return binascii.hexlify(signer.sign(h)).decode('ascii')
		
		
def display_transaction(transaction):
   #for transaction in transactions:
	dict = transaction.to_dict()
	print ("sender: " + dict['sender'])
	print ('-----')
	print ("recipient: " + dict['recipient'])
	print ('-----')
	print ("value: " + str(dict['value']))
	print ('-----')
	print ("time: " + str(dict['time']))
	print ('-----')
   

# for transaction in transactions:
	# display_transaction (transaction)
	# print ('--------------')
	
	
last_block_hash = ""

TPCoins = []
class Block:
	def __init__(self):
		self.verified_transactions = []
		self.previous_block_hash = ""
		self.Nonce = ""
		
	def dump_blockchain (self):
		print ("Number of blocks in the chain: " + str(len (self)))
		for x in range (len(TPCoins)):
			block_temp = TPCoins[x]
			print ("block # " + str(x))
			for transaction in block_temp.verified_transactions:
				display_transaction (transaction)
				print ('--------------')
			print ('=====================================')

t0 = Transaction (
   "Genesis",
   Dinesh.identity,
   500.0
)

block0 = Block()
block0.previous_block_hash = None
Nonce = None
block0.verified_transactions.append (t0)

digest = hash (block0)
last_block_hash = digest

TPCoins.append(block0)
dump_blockchain(TPCoins)


def mine(message, difficulty=1):
	assert difficulty >= 1
	prefix = '1' * difficulty
	for i in range(1000):
		digest = sha256(str(hash(message)) + str(i))
		if digest.startswith(prefix):
			print ("after " + str(i) + " iterations found nonce: "+ digest)
		return digest
		
last_transaction_index = 0