local class = require("class")
local TestClass = class.define("TestClass")

TestClass.member_field().a = 100
TestClass.static_field().b = 1980

TestClass.member_method("number", "string").print = function(self, age, name)
	print("member name: ".. name .. ", age: " .. age)
	self.a = 101
end

TestClass.virtual_method("number", "string").print = function(self, age, name)
	print("virtual name: ".. name .. ", age: " .. age)
	self.a = 101
end

TestClass.virtual_method("number", "number").print = function(self, age, height)
	print("age : " .. age .. ", height : " .. height)
end

TestClass.virtual_method("number", "string", "boolean").print = function(self, age, height, is_male)
	print("age : " .. age .. ", height : " .. height .. ", is male: " .. tostring(is_male))
end

TestClass.static_method().say_hello = function()
	print("hello")
end

--TestClass.member_method().

TestClass.end_define()

return TestClass