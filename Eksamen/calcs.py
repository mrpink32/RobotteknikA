import math as m
import numpy as np
types = set(["<rect", "<circle", "<ellipse", "<line", "<polyline", "<polygon", "<squircle", "<polynomial"])
file = open("C:/Users/theth/Desktop/GITHUB/RobotteknikA/Eksamen/data/test1.svg")
data = file.read().split()
def find(lst,word):
    for i in len(lst):
        if word in lst[i]:
            return i
    return -1

width = int(data[find(data,"width")].removeprefix('width="').removesuffix('"').removesuffix("mm"))
height = int(data[find(data,"height")].removeprefix('height="').removesuffix('"').removesuffix("mm"))

scale = 5/m.sqrt((width/2)**2 + (height/2)**2)

for i in len(data):
    if data[i] in types:
        if data[i] == "rect":
            x = scale*float(data[find(data,"x")].removeprefix('x="').removesuffix('"'))
            y = scale*float(data[find(data,"y")].removeprefix('y="').removesuffix('"'))
            w = scale*float(data[find(data,"width")].removeprefix('width="').removesuffix('"'))
            h = scale*float(data[find(data,"height")].removeprefix('height="').removesuffix('"'))
            t1 = h
            def vecx(t):
                return x
            def vecy(t):
                return t+y
            calc()
            
        
print(data)
def angle(x,y,n):
    n += 1
    a = m.sqrt(-15625*x**4 + (-31250*y**2 + 559925)*x**2 - 15625*y**4 + 273700*y**2 + 6780908)
    dicy[n].append(k*m.atan2(6752.971963 - 771.0280374*x**2 + 1.168224299*x*a - 771.0280374*y**2, 6.168224299 * a + (146.0280374*x**2 + 146.0280374*y**2 - 1278.971963)*x))
    if n < 3:
        angle(-x/2-m.sqrt(3)*y/2,-y/2+m.sqrt(3)*x/2,n)
n = 15
k = 1486/(2*m.pi)
def calc():
    x = []
    dicy = {}
    dicy[1] = []
    dicy[2] = []
    dicy[3] = []
    for i in range(1,n+1):
        x.append((t1+t1*m.cos((2*i-1)*m.pi/(2*n)))/2)
    for i in x:
        angle(vecx(i),vecy(i),0)
    p1 = np.polyfit(x,dicy[1],n-1)
    p2 = np.polyfit(x,dicy[2],n-1)
    p3 = np.polyfit(x,dicy[3],n-1)
    return (p1, p2, p3)