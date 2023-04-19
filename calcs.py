import math as m
import numpy as np
def angle(x,y,n):
    n += 1
    a = m.sqrt(-15625*x**4 + (-31250*y**2 + 559925)*x**2 - 15625*y**4 + 273700*y**2 + 6780908)
    dicy[n].append(k*m.atan2(6752.971963 - 771.0280374*x**2 + 1.168224299*x*a - 771.0280374*y**2, 6.168224299 * a + (146.0280374*x**2 + 146.0280374*y**2 - 1278.971963)*x))
    if n < 3:
        angle(-x/2-m.sqrt(3)*y/2,-y/2+m.sqrt(3)*x/2,n)
def vecx(t):
    return m.cos(t)
def vecy(t):
    return m.sin(t)
t0 = 0
t1 = 2*m.pi
n = 15
k = 1486/(2*m.pi)
x = []
dicy = {}
dicy[1] = []
dicy[2] = []
dicy[3] = []
for i in range(1,n+1):
    x.append((t0+t1+(t1-t0)*m.cos((2*i-1)*m.pi/(2*n)))/2)
for i in x:
    angle(vecx(i),vecy(i),0)
#print(x)
#print(dicy)

p1 = np.polyfit(x,dicy[1],n-1)
p2 = np.polyfit(x,dicy[2],n-1)
p3 = np.polyfit(x,dicy[3],n-1)
for i in p1:
    print(float(i))
print(' ')
for i in p2:
    print(float(i))
print(' ')
for i in p3:
    print(float(i))