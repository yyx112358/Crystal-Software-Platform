import cv2 as cv
import numpy as np
from matplotlib import pyplot as plt

print("如果DLL载入失败，请将PyCrystal.pyd和opencv_world420.dll复制到本文件路径")
import PyCrystal as cry

# help(cry)
# m=cv.imread(r"d:\Users\yyx11\Pictures\Mobile Wallpaper\gamersky_01origin_01_2018331194DF8.jpg")
# m=cv.resize(m,(540,960))
# cry.testim(m)
# cry.testim(cv.cvtColor(m,cv.COLOR_BGR2GRAY))
# j=cry.testreturn(m)
# cv.imshow('j',j)
# cv.waitKey()
# cv.imshow('',m)
# cv.waitKey()
# c=cry.Crystal()
# print(c)

manager = cry.CrystalSetManager()
manager.Load(r"d:\Users\yyx11\Desktop\Saved Pictures\first.crystalset")  # crystalset文件所在目录
assert (manager.IsOpen())
manager.SetDir(r"d:\Users\yyx11\Desktop\Saved Pictures\fig_batch1")  # 放fig_batch1所在目录

cnt = 0
avgTotalGray = []
avgCrystalGray = []
avgBackGray = []
indexs = []
while (manager.IsEnd() == False):
    ccs = manager.Get()  # 读取一张图片
    print(cnt)

    img = cv.cvtColor(ccs.OriginImage(), cv.COLOR_BGR2GRAY)
    # cv.imshow('pic',ccs.OriginImage())
    avgTotalGray.append(cv.mean(img)[0])

    indexs.append(cnt)
    cts = []
    for i in range(len(ccs)):
        crystal = ccs.get(i)  # 读取这张图片里被分割的第i个晶体
        cts.append(np.array(crystal.Contour()))
    mask = img & 0
    cv.drawContours(mask, cts, -1, (255), -1)
    avgCrystalGray.append(cv.mean(img, mask)[0])
    avgBackGray.append(cv.mean(img, ~mask)[0])
    # 	print(crystal,crystal.Contour())
    # cv.imshow('crystal', mask)
    # cv.waitKey(100)

    ccs.ReleaseImg()
    if cnt %100 == 0:
        plt.plot(indexs, avgTotalGray)
        plt.plot(indexs, avgCrystalGray)
        plt.plot(indexs, avgBackGray)
        plt.show()
    cnt += 1

plt.figure(figsize=(28, 18))
plt.plot(indexs, avgTotalGray)
plt.plot(indexs, avgCrystalGray)
plt.plot(indexs, avgBackGray)
plt.show()
