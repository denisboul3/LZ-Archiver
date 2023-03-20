import struct, ctypes, lz4.block, os

class entryStruct(ctypes.Structure):
	_fields_ = [
		('fileLength', ctypes.c_uint),
		('offset', ctypes.c_uint),
		('size', ctypes.c_uint),
		('name', ctypes.c_char*1024),
	]

class fKey(ctypes.Structure):
	_fields_ = [ ('magicKey', ctypes.c_uint) ]

structSize = ctypes.sizeof(entryStruct)


def StartReadingArchive(fileName, magicKey = -1):
	fileSize = os.path.getsize(fileName)
	
	offset = 4 # unsigned int

	with open(fileName, 'rb') as file:

		key = fKey.from_buffer_copy(file.read(offset))
		
		if key.magicKey != magicKey and magicKey != -1:
			print('Special key mismatch! {} {}'.format(key.magicKey, magicKey))
			return
			
		while offset < fileSize:
			
			# set file pointer to offset 4
			file.seek(offset)
			
			offset += structSize
			
			# set file pointer to offset 4 + structSize (which in our case is 1036)
			data = file.read(offset)
			
			# convert the data that we read into our struct
			entry = entryStruct.from_buffer_copy(data)
			
			# set file pointer to where the encrypted data is
			file.seek(offset)
			
			compressedBlock = file.read(entry.size)
			
			if entry.size != entry.fileLength:
				compressedBlock = lz4.block.decompress(compressedBlock, uncompressed_size = entry.fileLength + 1)

			os.makedirs(os.path.dirname(f'unpacked_{fileName.split(".")[0]}/{entry.name.decode()}'), exist_ok=True)
			open(f'unpacked_{fileName.split(".")[0]}/{entry.name.decode()}', 'wb').write(compressedBlock)
			
			print(f'Unpacking {entry.name.decode()}')
			
			offset += entry.size
			
		
fileName = input('Enter the desired file name: ')
magicKey = input('Enter the magic key (-1 to ignore(testing)): ')
StartReadingArchive(fileName, int(magicKey))