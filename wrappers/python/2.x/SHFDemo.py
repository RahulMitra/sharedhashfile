import SharedHashFile
from os import system, getpid
from struct import *

if __name__ == "__main__":

  SharedFileName = 'shf-test' + str(getpid()) 
  SharedHashFile.attach('/dev/shm', SharedFileName, 1)
  testQs         = 3
  testQItems     = 10
  testQItemSize  = 65535 # Maximum possible size of TCP packet
  testQidsNolockMax = 1
  SHF_QIID_NONE = 4294967295
  
  QueueMem = SharedHashFile.qNew(testQs, testQItems, testQItemSize, testQidsNolockMax)
  QueueFree = SharedHashFile.qNewName("QueueFree")
  QueueA2B = SharedHashFile.qNewName("QueueAtoB")
  QueueB2A = SharedHashFile.qNewName("QueueBtoA")

  i = 10

  print "Loading from Free Queue onto A2B Queue..."
  # Move items from free queue onto A2B queue
  currElem = SharedHashFile.qPullTailData(QueueFree)
  while (currElem is not None):
    # Get Data and Qiid
    currElemData = currElem[0]
    currElemQiid = currElem[1]

    # Write to data
    currElemData[0:4] = pack('i', i)

    # Push data onto A2B queue
    SharedHashFile.qPushHead(QueueA2B, currElemQiid)

    # Pull next item
    currElem = SharedHashFile.qPullTailData(QueueFree)
    i = i+1

  print "Reading from A2B Queue, Writing data, and loading onto B2A Queue..."

  # Move items from A2B queue onto B2A queue
  currElem = SharedHashFile.qPullTailData(QueueA2B)
  while (currElem is not None):
    # Get Data and Qiid
    currElemData = currElem[0]
    currElemQiid = currElem[1]

    # Read data
    data = currElemData[0:4]
    value = unpack('i', data)[0]
    print value

    # Write to data
    currElemData[0:4] = pack('i', value*2)

    # Push data onto B2A queue
    SharedHashFile.qPushHead(QueueB2A, currElemQiid)

    # Pull next item
    currElem = SharedHashFile.qPullTailData(QueueA2B)

  print "Reading from B2A Queue..."

  # Read items off B2A queue.
  currElem = SharedHashFile.qPullTailData(QueueB2A)
  while (currElem is not None):
    # Get Data and Qiid
    currElemData = currElem[0]
    currElemQiid = currElem[1]

    # Read data
    data = currElemData[0:4]
    value = unpack('i', data)[0]
    print value

    # Pull next item
    currElem = SharedHashFile.qPullTailData(QueueB2A)
