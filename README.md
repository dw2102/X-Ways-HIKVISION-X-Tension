  # X-Ways-HIKVISION-X-Tension
A X-Ways Forensics X-Tension to parse the HIKVISION filesystem 'HIK.2011.03.08'.

# Usage

After importing the filesystem image (E01, DD etc.) into a case rightclick on the virtual file which represents the unknown filesystem.
Load the X-Tension (HIKVISION.dll or whatever you want to name it) and execute it.

Now all video files should be visible in the volume snapshot.

For volumes (like DD images):

![volume](https://github.com/user-attachments/assets/7103869f-a124-4883-9e37-d3b682136f0d)

For disk images (physical drives or encase / E01):

![disk](https://github.com/user-attachments/assets/61cb577d-3dc7-479a-aa1f-931fc274d60a)

Video files named '19.01.2038 04:14:07' indicates a file which was not fully finished recording, but is still viewable.

I recommend to read the paper stated below to fully understand the filesystem.

Based on the paper from Jaehyeok Han, Doowon Jeong and Sangjin Lee (Korean University)

https://www.researchgate.net/publication/285429692_Analysis_of_the_HIKVISION_DVR_file_system

X-Tension is tested on version 21.4 SR-5

© 2025 Dane Wullen

NO WARRANTY, SOFWARE IS PROVIDED 'AS IS'
