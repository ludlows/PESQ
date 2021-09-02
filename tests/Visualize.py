
import matplotlib.pyplot as plt
import time
import csv
  
x = []
y = []

def update():
    with open('resultes/fmycsvfile.csv','r') as csvfile:
        lines = csv.reader(csvfile, delimiter=',')
        for row in lines:
            x.append(row[0])
            y.append(float(row[1]))
    
    plt.plot(x, y, color = 'g', linestyle = 'dashed',
            marker = 'o',label = "PESQ:")
    
    plt.xticks(rotation = 25)
    plt.xlabel('Dates')
    plt.ylabel('PASQ:(°C)')
    plt.title('P.862 Voice Quality Results', fontsize = 20)
    plt.grid()
    plt.legend()
    plt.draw()
    plt.show(block=False)
    input("hit[enter] to end.")
    plt.close('all')
while True:
    update()