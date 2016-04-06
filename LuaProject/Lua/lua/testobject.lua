local TestClass2 = require("testclass2")
local TestClass1 = require("testclass1")
local TestClass = require("testclass")

local test2object= TestClass2.new()

print(tostring(test2object))
local pp = test2object.print

print(tostring(test2object.print))
test2object:print(13, "yangcao1")


test2object:say_goodbye()
print(test2object.c)

test2object:print(13, "yangcao1")
test2object:print("yangcao", "yc")

local test1object = TestClass1.new()
test1object:print(15, "yc")

local testobject = TestClass.new()

local testobject1 = TestClass1.new()
testobject1.a = 102
print(testobject.a)

testobject:print(12, "yangcao")
testobject:print(16, "yangcao", true)


print(testobject.a)
print(testobject1.a)

_TestClass.b = 1888
print(_TestClass.b)
print(testobject.b)
print(testobject1.b)
testobject.b = 1989
testobject1.b = 1990
print(testobject.b)
print(testobject1.b)
print(test1object.b)

print(test1object.c)

test1object:say_hello()
test1object:say_goodbye()

test2object.a = 990

print(testobject.a)
print(testobject1.a)
print(test2object.a)

print(tostring(testobject))
print("")
print(tostring(testobject1))


