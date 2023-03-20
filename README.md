# LZ-Archiver

Usage: 

  Archiver.exe FolderName to encrypt a folder
  
  Archiver.exe FileName.extension to decrypt a crypted file
  
  Archiver.exe read FileName.extension FileInsideArchive => prints the raw data in console (TODO: dump into file)
  
  Python LZ4 Reader/app.py - self explanatory, just run it
  
  LZ4 Reader/ same as python archiver but not finished, the basic logic is implemented
  
  This has a possibility of being adapted to already an existing project that reads the data from text files either image file either binary files and much more. So instead reading the data from the unencrypted file it can read it from the encrypted file for reasons like security, avoid stealing your resources etc