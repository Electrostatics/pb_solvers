import numpy as np
import matplotlib.pyplot as plt
from pylab import *
from mpl_toolkits.mplot3d import Axes3D

from matplotlib import rcParams
rcParams.update({'figure.autolayout': True})

'''
Program to plot a 3D version of the ESP from PB-AM
'''
dirName='/Users/davidbrookes/'
# fileName = dirName + 'PBSAM/pb_solvers/pbam/'
fileName = dirName + 'data/2fgr/2fgr_tri_move_map.out'
outFile= dirName + 'Desktop/trimer_monoview_out.surf'

#-----------------------------------------------------------------------
def randrange(n, vmin, vmax):
    '''Get a random range of numbers'''
    return (vmax-vmin)*np.random.rand(n) + vmin

def FileOpen(fileName):
    """Gets data from 3D plot output of PB-AM"""
    lines = open(fileName).readlines()

    grid,org,dl = np.zeros(3), np.zeros(3), np.zeros(3)
    units, ct = 'jmol', 0
    pot = np.zeros((len(lines)-5, 4))

    for line in lines:
        temp = line.split()
        if 'units' in line[0:10]:
            units = temp[1]
        elif 'grid' in line[0:10]:
            grid[0], grid[1] = int(temp[1]), int(temp[2])
        elif 'origin' in line[0:10]:
            org[0], org[1] = float(temp[1]), float(temp[2])
        elif 'delta' in line[0:10]:
            dl[0], dl[1] = float(temp[1]), float(temp[2])
        elif '#' not in line:
            temp = [float(x) for x in line.split()]

            pot[ct][0] = temp[0]
            pot[ct][1] = temp[1]
            pot[ct][2] = temp[2]
            pot[ct][3] = temp[3]
            ct += 1

    return(pot, org, dl, units)

def dispPlot( org, bn, xv, yv, zv, potential,
                title = '', lege = '', outFile = None ):
    """Plots the colormap of potential plot, 3D"""
    fig = plt.figure(1, figsize = (5, 4));
    ax = fig.add_subplot(111,projection='3d')

    #colors = cm.bwr(potential) #(potential-min(potential))/
                                #(max(potential)-min(potential)))
    #colors.set_clim(vmin = min(potential),
                            #vmax = max(potential))

    colmap = cm.ScalarMappable(cmap=cm.jet)
    colmap.set_array(potential)

    n = len(xv)
    for i in range(n):
        if xv[i] < 0:
            break

    xv = xv[:i]
    yv = yv[:i]
    zv = zv[:i]
    potential=potential[:i]

    xv -= np.mean(xv)
    yv -= np.mean(yv)
    zv -= np.mean(zv)
    

    yg = ax.scatter(xv, yv, zv, c=potential, cmap = plt.get_cmap('jet'), marker='o', lw = 0)
    cb = fig.colorbar(colmap)
    cb.set_label("%s" % units)

    minl = min(min(xv), min(yv), min(zv))
    maxl = max(max(xv), max(yv), max(zv))

    ax.set_xlim([minl-10, maxl+10])
    ax.set_ylim([minl-10, maxl+10])
    ax.set_zlim([minl-10, maxl+10])

    plt.title(title, fontsize = 13);
    ax.set_xlabel(r'$X (\AA)$', fontsize = 12)
    ax.set_ylabel(r'$Y (\AA)$', fontsize = 12)
    ax.set_zlabel(r'$Z (\AA)$', fontsize = 12)
    for tick in ax.xaxis.get_major_ticks():
        tick.label.set_fontsize(8)
    for tick in ax.yaxis.get_major_ticks():
        tick.label.set_fontsize(8)
    for tick in ax.zaxis.get_major_ticks():
        tick.label.set_fontsize(8)
    if outFile != None:
        for ii in xrange(0,360,90):
            ax.view_init(elev=20., azim=ii)
            plt.gcf().subplots_adjust(bottom=0.15)
            plt.savefig(outFile+str(ii)+'.jpg', dpi = 300)

    # ax.xaxis._axinfo['label']['space_factor'] = 6
    plt.tight_layout()
    plt.close()
    #plt.show()

#------------------------------------------------------------------------------
# main

plt.close()
esp, org, dl, units = FileOpen(fileName)
if units == "jmol":
    units = "$J/$mol"
titl = 'Potential at surfaces in %s' % units

dispPlot( org, dl, esp[:,0], esp[:,1], esp[:,2], esp[:,3],
          titl, outFile=outFile)