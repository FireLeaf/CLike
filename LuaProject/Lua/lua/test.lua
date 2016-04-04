-- @Global deep copy 'src' table to 'dest' table
--	Copy value type data, create new table when copy each subtable
-- @Param dest the destination lua table
-- @Param src the source lua table
function deep_copy(dest, src)
	-- This table record already copied table, prevent circular reference
	local copied = {}

	-- @Local copy value or table, and check the table whether be copied
	local function _inner_copy(value)
		if type(value) ~= "table" then
			return value -- If the value is string,number,boolean,function just return value itself
		elseif copied[value] ~= nil then
			return copied[value] -- It's indicate the value table
		end

		local copy_value = {}
		-- Deep copy value table to new table
		for k, v in pairs(value) do
			copy_value[_inner_copy(k)] = _inner_copy(v)
		end

		copied[value] = copy_value

		return copy_value
	end

	for k, v in pairs(src) do
		dest[_inner_copy(k)] = _inner_copy(v)
	end
	return dest
end

a = {
	"hello",
	k = 10,
	hello = {},
}
b = deep_copy({}, a)
b.hello.c = 100
--print(a.helo.c)
print(b.hello.c)

print(a[1])
print(b[1])
a[1] = "world"
b[1] = "kitty"
print(a[1])
print(b[1])

-- @Global declare a global
-- @Param key global key, probably is a string, or a table¡¢function
-- @Param sugar must is "="
-- @Param value the global
declare_global = function ( key, sugar, value )
	if sugar ~= "=" then
		error([[Decalre global the second parameter must be "="! sorry! It's a sugar!]])
		return
	end
	rawset(_G, key, value)
end

setmetatable(_G, {__newindex = function(key, value) error(" Cannot indirect create global variable! use ") end})

declare_global("a", "=", 10)
print(a)

declare_global("b", "=", {})

local _value = {}

local self_meta =
{


	__newindex = function(tbl, key, value)
		_value[key] = value
	end,

	__index = function (tbl, key)
		return _value[key]
	end,
}
setmetatable(b, self_meta)
b.a = 10
print(b.a)
print(b.c)
b.a = 90
print(b.a)

declare_global("dd", "=", function()
	print("hello")
end)

dd()
