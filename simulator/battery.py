import operator
import numpy as np


def prod(iterable):
    return reduce(operator.mul, iterable, 1)

class batCell:
	#capacity in Ah#
	def __init__(self, voltage, cellCapacity, esr = 0.01):
		self.voltage = voltage
		self.maximumCapacity = cellCapacity + np.random.normal(0,cellCapacity/100.0)
		self.capacity = 1
		self.esr = esr + np.random.normal(0,esr*100)

	def __str__(self):
		return "%.2f" % (self.maximumCapacity*self.capacity) + "/" + "%.2f" % self.maximumCapacity

	# time in seconds
	def step(self, time,current):
		self.capacity -= time/3600.0*current/self.maximumCapacity
		assert self.capacity <= 1
		assert self.capacity >= 0


	def getVoltage(self):
		return self.voltage(self.capacity)



class module:
	def __init__(self, (s,p), drainCurrent, voltage, cellCapacity):
		self.size = (s,p)
		#cells is a tuple of a cell and a drain status
		self.cells = [( batCell(voltage, cellCapacity*p), False) for ser in range(s)]
		self.drainCurrent = drainCurrent
		self.esr = sum([cell[0].esr for cell in self.cells])

	def __str__(self):
		return "["+"]-[".join([cell[0].__str__() for cell in self.cells]) + "]"

	def setDrainStatus(self,drainList):
		for idx, cell in enumerate(self.cells):
			cell[1] = drainList[idx]

	def getCellVoltages(self):
		return [cell[0].getVoltage() for cell in self.cells]

	def getVoltage(self):
		return sum(self.getCellVoltages())

	def step(self, time, current):
		for cell in self.cells:
			cell[0].step(time, current+self.drainCurrent)

class pack:
	def __init__(self, (ms, mp), (s,p), drainCurrent, voltage, cellCapacity):
		self.size = (ms,mp)

		self.modules = [[module((s,p), drainCurrent, voltage, cellCapacity) for ser in range(ms)] for par in range(mp)]

	def __unicode__(self):
		frontDecider = lambda x: u'\u250f' if x == 0 else u'\u2517' if x == self.size[1]-1  else u'\u2523'
		endDecider = lambda x: u'\u2513\n' if x == 0 else u'\u251b' if x == self.size[1]-1  else u'\u252B\n'

		return u''.join( 
				[frontDecider(idx) + u"\u2770" + u"\u2771\u2501\u2501\u2770".join([module.__str__() for module in moduleSeries]) 
				+ u"\u2771" + endDecider(idx) for idx, moduleSeries in enumerate(self.modules)])

	def __str__(self):
	    return unicode(self).encode('utf-8')

	def computeDrains(self):
		#subclass to come up with strategies
		pass

	def step(self, time, current):
		self.computeDrains()

		seriesVoltages = [sum([module.getVoltage() for module in seriesModules ]) for seriesModules in self.modules]
		seriesResistances = [sum([module.esr for module in seriesModules ]) for seriesModules in self.modules]

		packVoltageNum = sum([seriesVoltages[i] * prod([seriesResistances[j] for j in range(self.size[1]) if i != j]) for i in range(self.size[1])])

		packVoltageDen = sum([prod([seriesResistances[j] for j in range(self.size[1]) if i != j]) for i in range(self.size[1])])

		packVoltage = packVoltageNum/packVoltageDen

		print packVoltage


		#todo, actually compute current through each with current divider
		for idx, moduleSeries in enumerate(self.modules):
			for module in moduleSeries:
				module.step(time, current/self.size[1] 
					- (packVoltage - seriesVoltages[idx])/seriesResistances[idx])

def fakeCellVoltage(capacity):
	return 3.3 - 3.3 * (1- capacity)


cell1 = batCell(fakeCellVoltage, 1)
print(cell1)
module1 = module((2,1), .01, fakeCellVoltage, 1)
print(module1)
module1.step(1,100)
print(module1)
pack1 = pack((4,2),(1,2),.005,fakeCellVoltage,1)
print(pack1)
for x in xrange(1,10):
	pack1.step(1,1000)
	print(pack1)
		