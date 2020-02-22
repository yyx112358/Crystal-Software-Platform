import cv2 as cv

print("如果DLL载入失败，请将PyCrystal.pyd和opencv_world420.dll复制到本文件路径")
import PyCrystal as cry

help(cry)
m=cv.imread(r"d:\Users\yyx11\Pictures\Mobile Wallpaper\gamersky_01origin_01_2018331194DF8.jpg")
m=cv.resize(m,(540,960))
cry.testim(m)
cry.testim(cv.cvtColor(m,cv.COLOR_BGR2GRAY))
j=cry.testreturn(m)
cv.imshow('j',j)
cv.waitKey()
cv.imshow('',m)
cv.waitKey()