import math
import numpy as np

types = set(["<rect", "<circle", "<ellipse", "<line", "<polyline", "<polygon", "<squircle", "<polynomial"])
CONVERSION_RATE = 1486/(2*math.pi)

def find(lst,word):
    for i in range(len(lst)):
        if word in lst[i]:
            return i
    return -1

def calc(prec, xt, dicy, t1 ,vecx, vecy):
    for i in range(1,prec+1):
        xt.append((t1+t1*math.cos((2*i-1)*math.pi/(2*prec)))/2)
    for i in xt:
        angle(vecx(i),vecy(i),0,dicy)
    #print(xt)
    #print(dicy[1])
    #print(dicy)
    p1 = np.polyfit(xt,dicy[1],prec-1)
    p2 = np.polyfit(xt,dicy[2],prec-1)
    p3 = np.polyfit(xt,dicy[3],prec-1)
    return (p1, p2, p3)

def angle(x,y,l,dicy):
    l += 1
    a = math.sqrt(-15625*x**4 + (-31250*y**2 + 559925)*x**2 - 15625*y**4 + 273700*y**2 + 6780908)
    #print('hi')
    dicy[l].append(CONVERSION_RATE*math.atan2(6752.971963 - 771.0280374*x**2 + 1.168224299*x*a - 771.0280374*y**2, 6.168224299 * a + (146.0280374*x**2 + 146.0280374*y**2 - 1278.971963)*x))
    if l < 3:
        angle(-x/2-math.sqrt(3)*y/2,-y/2+math.sqrt(3)*x/2,l,dicy)


class Svg_doc:
    height = 0
    width = 0
    scale = 0
    data = ""
    def __init__(self, path):
        file = open(path)
        self.data = file.read().split()

    def interpret(self):
        polynomial_lists = []
        self.width = int(self.data[find(self.data,"width")].removeprefix('width="').removesuffix('"').removesuffix("mm"))
        self.height = int(self.data[find(self.data,"height")].removeprefix('height="').removesuffix('"').removesuffix("mm"))
        self.scale = 5/math.sqrt((self.width/2)**2 + (self.height/2)**2)
        self.n = 15
        self.g = 0
        for i in range(len(self.data)):
            if self.data[i] == '<g':
                self.g = 1
            if self.data[i] == '</g>':
                self.g = 0
                break
            if self.data[i] in types and self.g == 1:
                if self.data[i] == "<rect":
                    rect = Rect(self.scale, self.width, self.height, self.data, i)
                    polynomial_lists.append(rect.shit())
                if self.data[i] == "<ellipse":
                    ellipse = Ellipse(self.scale, self.width, self.height, self.data, i, precision=self.n*4)
                    polynomial_lists.append(ellipse.shit())
                if self.data[i] == "<circle":
                    circle = Circle(self.scale, self.width, self.height, self.data, i, precision=self.n*4)
                    polynomial_lists.append(circle.shit())
        return polynomial_lists


class Rect:
    x = 0
    y = 0
    height = 0
    width = 0
    precision = 0
    xt = []
    dicy = {1:[],2:[],3:[]}
    #dicy[1] = []
    #dicy[2] = []
    #dicy[3] = []
    def __init__(self, scale, width, height, data, index, precision=15) -> None:
        self.precision = precision
        self.x = scale*float(data[index+find(data[index:],"x=")].removeprefix('x="').removesuffix('"')) - scale*width/2
        self.y = scale*float(data[index+find(data[index:],"y=")].removeprefix('y="').removesuffix('"')) - scale*height/2
        self.width = scale*float(data[index+find(data[index:],"width=")].removeprefix('width="').removesuffix('"'))
        self.height = scale*float(data[index+find(data[index:],"height=")].removeprefix('height="').removesuffix('"'))

    def shit(self):
        polynomial_list = []
        t1 = self.height
        def vecx(t):
            return self.x
        def vecy(t):
            return t+self.y
        polynomial_list.append(self.calc(t1,vecx,vecy))
        t1 = self.width
        def vecx(t):
            return self.x+t
        def vecy(t):
            return self.y+self.height
        polynomial_list.append(self.calc(t1,vecx,vecy))
        t1 = self.height
        def vecx(t):
            return self.x+self.width
        def vecy(t):
            return self.y+self.height-t
        polynomial_list.append(self.calc(t1,vecx,vecy))
        t1 = self.width
        def vecx(t):
            return self.x+self.width-t
        def vecy(t):
            return self.y
        polynomial_list.append(self.calc(t1,vecx,vecy))
        return polynomial_list

    def calc(self, t1 ,vecx, vecy):
        self.xt.clear()
        self.dicy = {1:[],2:[],3:[]}
        #dicy[1] = []
        #dicy[2] = []
        #dicy[3] = []
        for i in range(1,self.precision+1):
            self.xt.append((t1+t1*math.cos((2*i-1)*math.pi/(2*self.precision)))/2)
        for i in self.xt:
            self.angle(vecx(i),vecy(i),0)
        p1 = list(np.polyfit(self.xt,self.dicy[1],self.precision-1))
        p2 = list(np.polyfit(self.xt,self.dicy[2],self.precision-1))
        p3 = list(np.polyfit(self.xt,self.dicy[3],self.precision-1))
        return (p1, p2, p3)
    
    def angle(self,x,y,l):
        l += 1
        a = math.sqrt(-15625*x**4 + (-31250*y**2 + 559925)*x**2 - 15625*y**4 + 273700*y**2 + 6780908)
        self.dicy[l].append(CONVERSION_RATE*math.atan2(6752.971963 - 771.0280374*x**2 + 1.168224299*x*a - 771.0280374*y**2, 6.168224299 * a + (146.0280374*x**2 + 146.0280374*y**2 - 1278.971963)*x))
        if l < 3:
            self.angle(-x/2-math.sqrt(3)*y/2,-y/2+math.sqrt(3)*x/2,l)


class Ellipse:
    x = 0
    y = 0
    rx = 0
    ry = 0
    precision = 0
    xt = []
    dicy = {1:[],2:[],3:[]}
    #dicy[1] = []
    #dicy[2] = []
    #dicy[3] = []
    def __init__(self, scale, width, height, data, index, precision=15) -> None:
        self.precision = precision
        self.x = scale*float(data[index+find(data[index:],"cx=")].removeprefix('cx="').removesuffix('"')) - scale*width/2
        self.y = scale*float(data[index+find(data[index:],"cy=")].removeprefix('cy="').removesuffix('"')) - scale*height/2
        self.rx = scale*float(data[index+find(data[index:],"rx=")].removeprefix('rx="').removesuffix('"'))
        self.ry = scale*float(data[index+find(data[index:],"ry=")].removeprefix('ry="').removesuffix('"'))

    def shit(self) -> None:
        t1 = math.pi*(self.rx+self.ry)
        def vecx(t):
            return self.x+self.rx*math.cos(2*t/(self.rx+self.ry))
        def vecy(t):
            return self.y+self.ry*math.sin(2*t/(self.rx+self.ry))
        return [self.calc(t1,vecx,vecy)]

    def calc(self, t1 ,vecx, vecy):
        self.xt.clear()
        self.dicy = {1:[],2:[],3:[]}
        #dicy[1] = []
        #dicy[2] = []
        #dicy[3] = []
        for i in range(1,self.precision+1):
            self.xt.append((t1+t1*math.cos((2*i-1)*math.pi/(2*self.precision)))/2)
        for i in self.xt:
            self.angle(vecx(i),vecy(i),0)
        p1 = list(np.polyfit(self.xt,self.dicy[1],self.precision-1))
        p2 = list(np.polyfit(self.xt,self.dicy[2],self.precision-1))
        p3 = list(np.polyfit(self.xt,self.dicy[3],self.precision-1))
        return (p1, p2, p3)
    
    def angle(self,x,y,l):
        l += 1
        a = math.sqrt(-15625*x**4 + (-31250*y**2 + 559925)*x**2 - 15625*y**4 + 273700*y**2 + 6780908)
        self.dicy[l].append(CONVERSION_RATE*math.atan2(6752.971963 - 771.0280374*x**2 + 1.168224299*x*a - 771.0280374*y**2, 6.168224299 * a + (146.0280374*x**2 + 146.0280374*y**2 - 1278.971963)*x))
        if l < 3:
            self.angle(-x/2-math.sqrt(3)*y/2,-y/2+math.sqrt(3)*x/2,l)


class Circle:
    x = 0
    y = 0
    r = 0
    xt = []
    dicy = {1:[],2:[],3:[]}
    #dicy[1] = []
    #dicy[2] = []
    #dicy[3] = []
    def __init__(self, scale, width, height, data, index, precision=15) -> None:
        self.precision = precision
        self.x = scale*float(data[index+find(data[index:],"cx=")].removeprefix('cx="').removesuffix('"')) - scale*width/2
        self.y = scale*float(data[index+find(data[index:],"cy=")].removeprefix('cy="').removesuffix('"')) - scale*height/2
        self.r = scale*float(data[index+find(data[index:],"r=")].removeprefix('r="').removesuffix('"'))

    def shit(self):
        t1 = 2*math.pi*self.r
        def vecx(t):
            return self.x+self.r*math.cos(t/self.r)
        def vecy(t):
            return self.y+self.r*math.sin(t/self.r)
        return [self.calc(t1,vecx,vecy)]

    def calc(self, t1 ,vecx, vecy):
        self.xt.clear()
        self.dicy = {1:[],2:[],3:[]}
        #dicy[1] = []
        #dicy[2] = []
        #dicy[3] = []
        for i in range(1,self.precision+1):
            self.xt.append((t1+t1*math.cos((2*i-1)*math.pi/(2*self.precision)))/2)
        for i in self.xt:
            self.angle(vecx(i),vecy(i),0)
        p1 = list(np.polyfit(self.xt,self.dicy[1],self.precision-1))
        p2 = list(np.polyfit(self.xt,self.dicy[2],self.precision-1))
        p3 = list(np.polyfit(self.xt,self.dicy[3],self.precision-1))
        return (p1, p2, p3)
    
    def angle(self,x,y,l):
        l += 1
        a = math.sqrt(-15625*x**4 + (-31250*y**2 + 559925)*x**2 - 15625*y**4 + 273700*y**2 + 6780908)
        self.dicy[l].append(CONVERSION_RATE*math.atan2(6752.971963 - 771.0280374*x**2 + 1.168224299*x*a - 771.0280374*y**2, 6.168224299 * a + (146.0280374*x**2 + 146.0280374*y**2 - 1278.971963)*x))
        if l < 3:
            self.angle(-x/2-math.sqrt(3)*y/2,-y/2+math.sqrt(3)*x/2,l)

def main():
    svg = Svg_doc("RobotteknikA/Eksamen/data/test2.svg")
    svg.interpret()


if __name__ == "__main__":
    main()