import math as m
import numpy as np
types = set(["<rect", "<circle", "<ellipse", "<line", "<polyline", "<polygon", "<squircle", "<polynomial"])
file = open("C:/Users/theth/Desktop/GITHUB/RobotteknikA/Eksamen/data/test1.svg")
data = file.read().split()
def find(lst,word):
    for i in range(len(lst)):
        if word in lst[i]:
            return i
    return -1
n = 15
k = 1486/(2*m.pi)
width = int(data[find(data,"width")].removeprefix('width="').removesuffix('"').removesuffix("mm"))
height = int(data[find(data,"height")].removeprefix('height="').removesuffix('"').removesuffix("mm"))

scale = 5/m.sqrt((width/2)**2 + (height/2)**2)


def calc():
    xt = []
    dicy = {}
    dicy[1] = []
    dicy[2] = []
    dicy[3] = []
    for i in range(1,n+1):
        xt.append((t1+t1*m.cos((2*i-1)*m.pi/(2*n)))/2)
    for i in xt:
        angle(vecx(i),vecy(i),0)
    print(xt)
    print(dicy[1])
    print(dicy)
    p1 = np.polyfit(xt,dicy[1],n-1)
    p2 = np.polyfit(xt,dicy[2],n-1)
    p3 = np.polyfit(xt,dicy[3],n-1)
    return (p1, p2, p3)

def angle(x,y,l):
    l += 1
    a = m.sqrt(-15625*x**4 + (-31250*y**2 + 559925)*x**2 - 15625*y**4 + 273700*y**2 + 6780908)
    print('hi')
    dicy[l].append(k*m.atan2(6752.971963 - 771.0280374*x**2 + 1.168224299*x*a - 771.0280374*y**2, 6.168224299 * a + (146.0280374*x**2 + 146.0280374*y**2 - 1278.971963)*x))
    if l < 3:
        angle(-x/2-m.sqrt(3)*y/2,-y/2+m.sqrt(3)*x/2,l)

print(data)
g = 0
for i in range(len(data)):
    if data[i] == '<g':
        g = 1
    if data[i] == '</g>':
        g = 0
    if data[i] in types and g == 1:
        if data[i] == "<rect":
            x = scale*float(data[i+find(data[i:],"x=")].removeprefix('x="').removesuffix('"')) - scale*width/2
            y = scale*float(data[i+find(data[i:],"y=")].removeprefix('y="').removesuffix('"')) - scale*height/2
            w = scale*float(data[i+find(data[i:],"width=")].removeprefix('width="').removesuffix('"'))
            h = scale*float(data[i+find(data[i:],"height=")].removeprefix('height="').removesuffix('"'))
            print(x,y,w,h)
            t1 = h
            def vecx(t):
                return x
            def vecy(t):
                return t+y
            calc()
            t1 = w
            def vecx(t):
                return x+t
            def vecy(t):
                return y+h
            calc()
            t1 = h
            def vecx(t):
                return x+w
            def vecy(t):
                return y+h-t
            calc()
            t1 = w
            def vecx(t):
                return x+w-t
            def vecy(t):
                return y
            calc()