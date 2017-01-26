import subprocess
import os

def cacheSize(lowerBound, upperBound):
    ret = [lowerBound]
    while ret[-1] < upperBound:
        ret.append(ret[-1]*2)
    return ret


cache_sizes = cacheSize(256,8192)
block_sizes = cacheSize(32,256)

cases = [(x,y,z) for x in cache_sizes for y in block_sizes for z in ['DM','FIFO','LRU']]

for testCase in cases:
    print('./sim ' + str(testCase[0]) + ' ' + str(testCase[1]) + ' ' + str(testCase[2]) + ' > output.txt')
    #os.system('./sim2 ' + str(testCase[0]) + ' ' + str(testCase[1]) + ' ' + str(testCase[2]) + ' > output2.txt')
    #var = subprocess.check_output('diff output.txt output2.txt', shell = True)
    #if var == '':
     #   print str(testCase) + 'OK!'
    #else:
     #   print str(testCase) + 'NOT OK!'
      #  break
