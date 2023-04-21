import math
import numpy as np

types = set(["<rect", "<circle", "<ellipse", "<line", "<polyline", "<polygon", "<squircle", "<polynomial"])
CONVERSION_RATE = 1486/(2*math.pi)
PRECISION = 15
POS_PER_SEC = 10

def find(lst,word):
    for i in range(len(lst)):
        if word in lst[i]:
            return i
    return -1


class SvgHandler:
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
                    ellipse = Ellipse(self.scale, self.width, self.height, self.data, i)
                    polynomial_lists.append(ellipse.shit())
                if self.data[i] == "<circle":
                    circle = Circle(self.scale, self.width, self.height, self.data, i)
                    polynomial_lists.append(circle.shit())
                if self.data[i] == "<line":
                    line = Line(self.scale, self.width, self.height, self.data, i)
                    polynomial_lists.append(line.shit())
                if self.data[i] == "<polyline":
                    polyline = Polyline(self.scale, self.width, self.height, self.data, i)
                    polynomial_lists.append(polyline.shit())
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
    def __init__(self, scale, width, height, data, index) -> None:
        self.precision = PRECISION
        self.x = scale*float(data[index+find(data[index:],"x=")].removeprefix('x="').removesuffix('"')) - scale*width/2
        self.y = scale*float(data[index+find(data[index:],"y=")].removeprefix('y="').removesuffix('"')) - scale*height/2
        self.width = scale*float(data[index+find(data[index:],"width=")].removeprefix('width="').removesuffix('"'))
        self.height = scale*float(data[index+find(data[index:],"height=")].removeprefix('height="').removesuffix('"'))

    def shit(self) -> list[list[list[float]]]:
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
    def __init__(self, scale, width, height, data, index) -> None:
        self.precision = 4*PRECISION
        self.x = scale*float(data[index+find(data[index:],"cx=")].removeprefix('cx="').removesuffix('"')) - scale*width/2
        self.y = scale*float(data[index+find(data[index:],"cy=")].removeprefix('cy="').removesuffix('"')) - scale*height/2
        self.rx = scale*float(data[index+find(data[index:],"rx=")].removeprefix('rx="').removesuffix('"'))
        self.ry = scale*float(data[index+find(data[index:],"ry=")].removeprefix('ry="').removesuffix('"'))

    def shit(self) -> list[list[list[float]]]:
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
    def __init__(self, scale, width, height, data, index) -> None:
        self.precision = 4*PRECISION
        self.x = scale*float(data[index+find(data[index:],"cx=")].removeprefix('cx="').removesuffix('"')) - scale*width/2
        self.y = scale*float(data[index+find(data[index:],"cy=")].removeprefix('cy="').removesuffix('"')) - scale*height/2
        self.r = scale*float(data[index+find(data[index:],"r=")].removeprefix('r="').removesuffix('"'))

    def shit(self) -> list[list[list[float]]]:
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

class Line:
    x1 = 0
    y1 = 0
    x2 = 0
    y2 = 0
    precision = 0
    xt = []
    dicy = {1:[],2:[],3:[]}
    #dicy[1] = []
    #dicy[2] = []
    #dicy[3] = []
    def __init__(self, scale, width, height, data, index) -> None:
        self.precision = PRECISION
        self.x1 = scale*float(data[index+find(data[index:],"x1=")].removeprefix('x1="').removesuffix('"')) - scale*width/2
        self.y1 = scale*float(data[index+find(data[index:],"y1=")].removeprefix('y1="').removesuffix('"')) - scale*height/2
        self.x2 = scale*float(data[index+find(data[index:],"x2=")].removeprefix('x2="').removesuffix('"')) - scale*width/2
        self.y2 = scale*float(data[index+find(data[index:],"y2=")].removeprefix('y2="').removesuffix('"')) - scale*height/2

    def shit(self) -> list[list[list[float]]]:
        t1 = math.sqrt((self.x2-self.x1)^2+(self.y2-self.y1)^2)
        def vecx(t):
            return self.x1 + t * (self.x2 - self.x1) / t1
        def vecy(t):
            return self.y1 + t * (self.y2 - self.y1) / t1
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

class Polyline:
    X = []
    Y = []
    precision = 0
    xt = []
    dicy = {1:[],2:[],3:[]}
    #dicy[1] = []
    #dicy[2] = []
    #dicy[3] = []
    def __init__(self, scale, width, height, data, index) -> None:
        self.precision = PRECISION
        self.place = index+find(data[index:],"points=")
        a,b = map(float, data[self.place].removeprefix('points="').split(","))
        self.X.append(a)
        self.Y.append(b)
        while True:
            self.place += 1
            if data[self.place][-1] == '"':
                a,b = map(float, data[self.place].removesuffix('"').split(","))
                self.X.append(a)
                self.Y.append(b)
                break
            a,b = map(float, data[self.place].split(","))
            self.X.append(a)
            self.Y.append(b)


    def shit(self) -> list[list[list[float]]]:
        polynomial_list = []
        for i in range(len(self.X)-1):
            t1 = math.sqrt((self.X[i+1]-self.X[i])^2+(self.Y[i+1]-self.Y[i])^2)
            def vecx(t):
                return self.X[i] + t * (self.X[i+1] - self.X[i]) / t1
            def vecy(t):
                return self.Y[i] + t * (self.Y[i+1] - self.Y[i]) / t1
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

class Polygon:
    X = []
    Y = []
    precision = 0
    xt = []
    dicy = {1:[],2:[],3:[]}
    #dicy[1] = []
    #dicy[2] = []
    #dicy[3] = []
    def __init__(self, scale, width, height, data, index) -> None:
        self.precision = PRECISION
        self.place = index+find(data[index:],"points=")
        a,b = map(float, data[self.place].removeprefix('points="').split(","))
        self.X.append(a)
        self.Y.append(b)
        self.X.append(self.X[0])
        self.Y.append(self.Y[0])
        while True:
            self.place += 1
            if data[self.place][-1] == '"':
                a,b = map(float, data[self.place].removesuffix('"').split(","))
                self.X.append(a)
                self.Y.append(b)
                break
            a,b = map(float, data[self.place].split(","))
            self.X.append(a)
            self.Y.append(b)


    def shit(self) -> list[list[list[float]]]:
        polynomial_list = []
        for i in range(len(self.X)-1):
            t1 = math.sqrt((self.X[i+1]-self.X[i])^2+(self.Y[i+1]-self.Y[i])^2)
            def vecx(t):
                return self.X[i] + t * (self.X[i+1] - self.X[i]) / t1
            def vecy(t):
                return self.Y[i] + t * (self.Y[i+1] - self.Y[i]) / t1
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

def main():
    svg = SvgHandler("C:/Users/theth/Desktop/GITHUB/RobotteknikA/Eksamen/data/test2.svg")
    svg.interpret()


if __name__ == "__main__":
    main()
