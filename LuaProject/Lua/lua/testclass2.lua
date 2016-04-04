local class = require("class")
class.declare("TestClass1")
class.declare("TestClass")

local TestClass2 = class.define("TestClass2", "TestClass1", "TestClass")

TestClass2.static_field().e = 7655

TestClass2.virtual_method().say_goodbye = function()
	print("good bye2")
end

TestClass2.virtual_method("string", "string").print = function(self, boy, girl)
	print("boy name: ".. boy .. ", age: " .. girl)
	self.a = 101888
end

TestClass2.end_define()

return TestClass2