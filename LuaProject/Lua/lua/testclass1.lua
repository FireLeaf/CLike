local class = require("class")
class.declare("TestClass")

local TestClass1 = class.define("TestClass1", "TestClass")

TestClass1.static_field().c = 7655

TestClass1.virtual_method().say_goodbye = function()
	print("good bye")
end

TestClass1.end_define()

return TestClass1