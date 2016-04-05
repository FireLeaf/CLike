--
-- Author: OwlGame
-- Date: 2016-04-04 09:44:17
--

--module("class")

-- @Global shadow copy 'src' table to 'dest' table
--	Only copy table reference, and value type data
-- @Param dest the destination lua table
-- @Param src the source lua table
function shadow_copy(dest, src)
	for k, v in pairs(src) do
		if dest[k] ~= v then
			dest[k] = v
		end
	end
end

-- @Global shadow copy without metatable 
function shadow_copy_withou_meta(dest, src)
	local metatb = getmetatable(dest)
	shadow_copy(dest, src)
	setmetatable(dest, metatb)
end

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

-- @Global trait data type
-- @Param v data
function trait_type( v )
	local data_type = type(v)
	if data_type == "nil" then -- If pass nil, we set it's a userdata type
		data_type = "userdata"
	elseif data_type == "userdata" then
		if not v.native_type and type(v.native_type) == "function " then
			return v.native_type()
		end
	end

	return data_type
end

-- @Global declare a global
-- @Param key global key, probably is a string, or a table、function
-- @Param sugar must is "="
-- @Param value the global
function declare_global( key, sugar, value )
	if sugar ~= "=" then
		error([[Decalre global the second parameter must be "="! sorry! It's a sugar!]])
		return
	end
	rawset(_G, key, value)
	return value
end

setmetatable(_G, {__newindex = function(key, value) error(" Cannot indirect create global variable! use ") end})

-- @Table store virtual function,
local class_meta = {
	class_name = "",
	is_inherit_attr = false, -- when create first this class object, copy parent static field and all method to this
	is_defined = false, -- indicate whether class has defined

	virtual_method = {}, -- virtual function, implement dynamic binding, object space
	member_method = {}, -- member function, object space
	static_method = {}, -- static function, class space

	member_field = {}, -- member field, object space
	static_field = {}, -- static field, class space

	inherit_classes = {}, -- inherit_classes
}

-- @Global declare global variable, like class = {}
declare_global("class", "=", {})

class.class_meta_list = {} --
class.create_factory = {}
class.template_list = {}

-- @Function delcare a class
class.declare = function (class_name)
	local meta = class.class_meta_list[class_name]
	if meta ~= nil then
		return meta
	end

	-- Create a new meta
	meta = deep_copy({}, class_meta)
	meta.class_name = class_name
	class.class_meta_list[class_name] = meta

	-- Declare global class named
	local class_static = declare_global("_"..class_name, "=", {})
	setmetatable(class_static, {
			__newindex = function(self, key, value)
				local updated = false

				-- @Local update static field
				local function _update_static(meta_info)
					local value_ref = meta_info.static_field[key]
					if value_ref then
						rawset(meta_info.static_field, key, value)
						return true
					end

					for i = 1, #meta_info.inherit_classes do
						local is_update = _update_static(meta_info.inherit_classes[i])
						if is_update then
							return true
						end
					end
				end

				updated = _update_static(meta)
				if not updated then
					error("Static class cannot add attrbute!")
				end
			end,
			__index = function(self, key)
				--[[print("access static filed " .. key)
				for i = 1,  #meta.static_field do
					print(meta.static_field[i])
				end]]
				-- @Local find static method
				local function _find_static(meta_info)
					-- Fisrst in static field
					local value_ref = meta.static_field[key]
					if value_ref then return value_ref end

					-- Second in static method
					value_ref = meta.static_method[key]
					if value_ref then return value_ref end

					for i = 1, #meta_info.inherit_classes do
						value_ref = _find_static(meta_info.inherit_classes[i])
						if value_ref then return value_ref end
					end
					return nil
				end

				return _find_static(meta)
			end
		})
	return meta
end

-- @Function
class.define = function (class_name, ...)

	local class_template = {}
	local meta = class.declare(class_name)
	if meta.is_defined then
		error("Redefinition class '" .. class_name .. "'")
	end

	-- Parse inherit list
	for i = 1, select('#', ...) do
		local inherit_name = select(i, ...)
		if not inherit_name or type(inherit_name) ~= "string" or not class.class_meta_list[inherit_name] then
			error("Class '".. class_name .. "' inherit class error in define! It may inherit name not a string or not decalared inherit class!")
		end
		--print(inherit_name)
		meta.inherit_classes[#meta.inherit_classes + 1] = class.class_meta_list[inherit_name]
	end

	--print(#meta.inherit_classes)
	
	-- @Local assemble argument descriptions
	local function _assemble_arg_desces(arg_desces)
		local parameter_list_type = "(self"
		for i = 1, #arg_desces do
			parameter_list_type = parameter_list_type .. "," .. arg_desces[i]
		end
		parameter_list_type = parameter_list_type .. ")"
		return parameter_list_type
	end

	-- Generate object create factory
	class.create_factory["create_" .. class_name] = function ( ... )
		if not meta.is_defined then
			error("Class :" .. class_name .. " not defined! Please end define it!")
		end

		if not meta.is_inherit_attr then
			--print("dsfnansjn")
			-- Copy parent
			local __copied_class = {}
			-- @Local copy all parent method or static field to this meta
			local function _copy_attribute( attrs, attr_desc, meta_info)

				-- If already build inherit donnot build again,
				--	here should be optimized : if parent not build herit, help parent build it
				if not meta_info.is_inherit_attr then
					for i = 1, #meta_info.inherit_classes do
						--print("ddd")
						local parent_meta_info = meta_info.inherit_classes[i]
						assert(parent_meta_info.is_defined)
						if __copied_class[parent_meta_info] == nil then
							_copy_attribute(attrs, attr_desc, parent_meta_info)
						end
					end
				end

				__copied_class[meta_info] = true

				-- @Local find same argument function
				local function _is_same_arg_func(func1, func2)
					if #func1.arg_desces ~= #func2.arg_desces then
						return false
					else
						for arg_idx = 1, #func1.arg_desces do
							if func1.arg_desces[arg_idx] ~= func2.arg_desces[arg_idx] then
								return false
							end
						end
					end

					return true
				end

				-- @Local copy and check method
				local function _check_copy_attribute(method_name, methods)
					local have_attribute = (nil ~= attrs[attr_desc])
					attrs[attr_desc][method_name] = attrs[attr_desc][method_name] or {}
					local attrs_methods = attrs[attr_desc][method_name]
					--print(method_name .. " : " .. #methods)
					local replaced_or_added = {} -- save method replaced or added idx
					for method_idx = 1, #methods do
						local _method = methods[method_idx]
						if not have_attribute then -- Direct insert, because current not this name method
							print("get a ".. attr_desc .. ":" .. method_name .." from class:" .. meta_info.class_name .. _assemble_arg_desces(_method.arg_desces))
							attrs_methods[#attrs_methods] = _method
						else
							local inserted = false
							for func_idx = 1, #attrs_methods do
								if not replaced_or_added[func_idx] then
									local _func = attrs_methods[func_idx]
									if _is_same_arg_func(_func, _method) then
										inserted = true
										replaced_or_added[func_idx] = true
									end
								end
							end

							if not inserted then
								print("get A ".. attr_desc .. ":" .. method_name .." from class:" .. meta_info.class_name .. _assemble_arg_desces(_method.arg_desces))
								attrs_methods[#attrs_methods + 1] = _method
								replaced_or_added[#attrs_methods] = true
							end
						end
					end
				end

				print("ccc " .. attr_desc)
				for k,v in pairs(meta_info[attr_desc]) do
					
					_check_copy_attribute(k, v)
				end
			end

			local temp_simple_meta = {virtual_method={}, member_method={}, static_method={}}
			_copy_attribute(temp_simple_meta, "virtual_method", meta)
			_copy_attribute(temp_simple_meta, "member_method", meta)
			_copy_attribute(temp_simple_meta, "static_method", meta)
			--print("v:" .. #temp_simple_meta.virtual_method .. " m:" .. #temp_simple_meta.member_method .. " s:" .. #temp_simple_meta.static_method)
			shadow_copy_withou_meta(meta.virtual_method , temp_simple_meta.virtual_method)
			--print("v:" ..#meta.virtual_method)
			shadow_copy_withou_meta(meta.member_method , temp_simple_meta.member_method)
			shadow_copy_withou_meta(meta.static_method , temp_simple_meta.static_method)

			--_copy_attribute(meta.static_field, "stat")
			meta.is_inherit_attr = true
		end

		local object = {}
		--object.meta.virtual_method = shadow_copy({}, meta.virtual_method)
		--object.meta.member_method = shadow_copy({}, meta.member_method)
		--object.meta.static_method = shadow_copy({}, meta.static_method)

		-- Construct object member field

		object.meta = {}

		-- Copy member field
		local __copied_field_calss = {}
		-- @Local copy member_field from meta_info
		local function _copy_member_filed(field, meta_info)
			--print(#meta_info.inherit_classes)
			-- First copy parent
			for i = 1, #meta_info.inherit_classes do
				local parent_meta_info = meta_info.inherit_classes[i]
				if __copied_field_calss[parent_meta_info] == nil then
					_copy_member_filed(field, parent_meta_info)
				end
			end

			__copied_field_calss[meta_info] = true
			-- If parent member field conflict
			deep_copy(field, meta_info.member_field)
			return field
		end
		local object_member_field = _copy_member_filed({}, meta)
		object.meta.member_field = object_member_field

		--deep_copy(object_member_field, meta.member_field)

		local object_metatable = {
			__tostring = function()
				-- @Local print function table
				local function _print_function_table(pre, method_list)
					for k, v in pairs(method_list) do
						for method_idx = 1, #v do
							local str_desc = _assemble_arg_desces(v[method_idx].arg_desces)
							str_desc = pre .. k .. str_desc
							print(str_desc)
						end
					end
				end
				_print_function_table("virutal ", meta.virtual_method)
				_print_function_table("member ", meta.member_method)
				_print_function_table("static ", meta.static_method)
				return "end"
			end,
			__newindex = function(self, key, value)
				local updated = false

				-- @Local find attribute
				local function _update_attribute(key, value)
					-- @Local find extra attribute
					local function _update_extra_attribute(meta_info, key, value)
						-- Find in static field
						local value_ref = meta_info.static_field[key]
						if value_ref then
							rawset(meta_info.static_field, key, value)
							return true
						end

						for i = 1, #meta_info.inherit_classes do
							local is_update = _update_extra_attribute(meta_info.inherit_classes[i], key, value)
							if is_update then return true end
						end
						return false
					end

					-- Find in member field
					local value_ref = object.meta.member_field[key]
					if value_ref then
						rawset(object.meta.member_field, key, value)
						return true
					end

					return _update_extra_attribute(meta, key, value)
				end
				updated = _update_attribute(key, value)
				if not updated then
					error("Object forbid create new attribute!")
				end
			end,
			__index = function(self, key)
				-- @Local find attribute
				local function _find_attribute()
					-- @Local find extra attribute
					local function _find_extra_attribute(meta_info)
						if not meta_info.is_defined then
							error("class not defined [" .. meta_info.class_name .. "]")
						end
						-- Find in member field
						local value_ref = meta_info.member_method[key]
						if value_ref then return value_ref end

						-- Find in static field
						value_ref = meta_info.static_field[key]
						if value_ref then return value_ref end

						-- Find in static method
						value_ref = meta_info.static_method[key]
						if value_ref then return value_ref end

						for i = 1, #meta_info.inherit_classes do
							--print("find parent")
							value_ref = _find_extra_attribute(meta_info.inherit_classes[i])
							if value_ref then return value_ref end
						end
						return nil
					end

					-- Find in member field
					local value_ref = object.meta.member_field[key]
					if value_ref then return value_ref end

					-- First find in virtual table
					local value_ref = meta.virtual_method[key]
					if value_ref then return value_ref end

					return value_ref or _find_extra_attribute(meta)
				end

				local ret_ref = _find_attribute(key)
				if ret_ref then
					return ret_ref
				else
					error("access no exist field :[" .. key .. "]")
				end
			end
		}

		setmetatable(object, object_metatable)

		return object
	end


	class_template.end_define = function ()
		meta.is_defined = true
	end

	class_template.method = function(method_type,...)
		if meta.is_defined then
			error("Already defined class [ " .. class_name .."]")
		end

		local method_wrap = {}
		local arg_desces = {...}
		local method_list

		--[[local parameter_list_type = "(self"
		for i = 1, #arg_desces do
			parameter_list_type = parameter_list_type .. "," .. arg_desces[i]
		end
		parameter_list_type = parameter_list_type .. ")"
		print(" parameter list :" .. parameter_list_type)]]

		-- Find match method list
		if method_type == "v" then
			method_list = meta.virtual_method
		elseif method_type == "m" then
			method_list = meta.member_method
		elseif method_type == "s" then
			method_list = meta.static_method
		else
			error("unkonwn method type")
		end

		setmetatable(method_wrap, {
			__newindex = function ( self, key, value )
				if type(value) ~= "function" then
					error("Value is not a function")
				end

				assert(type(key) == "string")

				local methods = method_list[key]

				-- Create a method table if necessary
				if not methods then
					methods = {}
					method_list[key] = methods
					setmetatable(methods, {
							__call = function(self, ...)
								-- @Local check argument
								local function _check_argument(ca, ...)
									--print(#ca.arg_desces.. ":" .. select('#', ...))
									if #ca.arg_desces ~= (select('#', ...) - 1) then
										return
									end

									for i = 1, #ca.arg_desces do
										local arg = select(i + 1, ...)
										if ca.arg_desces[i] ~= trait_type(arg) then
											--print()
											return false
										end
									end
									return true
								end

								-- Find a best function match parameter to call
								for i = 1, #methods do
									local have_call = false
								 	local callable = methods[i]
								 	if _check_argument(callable, ...) then
										--print("hello")
								 		return callable(...)
								 	end
								end

								local parameter_list_type = "(self"
								for i = 2, select('#', ...) do
									local arg = select(i, ...)
									parameter_list_type = parameter_list_type .. "," .. trait_type(arg)
								end
								parameter_list_type = parameter_list_type .. ")"
								error("class [" .. class_name .. "] no method [" .. key .. "] parameter type :" .. parameter_list_type)
							end
						})
				end


				-- @Local check method whether redefination
				local function _check_redefine(methods, arg_desces)
					if #methods == 0 then
						return false
					end
					local all_same = true
					for i = 1, #methods do
						local ca = methods[i]
						all_same = true
						if #ca.arg_desces ~= #arg_desces then
							all_same = false
						else
							for arg_idx = 1, #ca.arg_desces do
								if ca.arg_desces[arg_idx] ~= arg_desces[arg_idx] then
									all_same = false
									break
								end
							end
						end

						if all_same then break end
					end
					return all_same
				end

				if _check_redefine(methods, arg_desces) then

					error("Class[".. class_name .."] Function [" .. key .."] redefinitation" .. " parameter list :" .. _assemble_arg_desces(arg_desces))
				end

				local callable = {}
				callable.arg_desces = arg_desces
				callable.func = value
				setmetatable(callable, {__call = function(self, ...)
					-- @Local check argument
					local function _check_argument(ca, ...)
						assert(#ca.arg_desces == (select('#', ...) - 1))
						for i = 1, #ca.arg_desces do
							if ca.arg_desces[i] ~= trait_type(select(i + 1, ...)) then
								error("Class[".. class_name .."] Function [" .. key .."] call argument[" .. i .. "] unmatch" )
							end
						end
					end
					_check_argument(callable, ...)
					return callable.func(...)
				end})

				-- Record this callable
				methods[#methods + 1] = callable

			end
			})
		return method_wrap
	end

	class_template.virtual_method = function (...)
		return class_template.method("v", ...)
	end

	class_template.member_method = function(...)
		return class_template.method("m", ...)
	end

	class_template.static_method = function(...)
		return class_template.method("s", ...)
	end

	class_template.field = function ( field_type, ... )
		if meta.is_defined then
			error("Already defined class [ " .. class_name .."]")
		end

		local field_wrap = {}
		local filed_list
		if field_type == "m" then
			filed_list = meta.member_field
		elseif field_type == "s" then
			filed_list = meta.static_field
			--print("heleoinj")
		else
			error("Unkonw filed type : ".. field_type)
		end
		setmetatable(field_wrap, {
				__newindex = function(self, key, value)
					filed_list[key] = value
				end
			})
		return field_wrap
	end

	class_template.member_field = function()
		return class_template.field("m")
	end

	class_template.static_field = function ()
		return class_template.field("s")
	end

	class_template.new = function()
		return class.create_factory["create_" .. class_name]()
	end

	return class_template
end

setmetatable(class, {
		__newindex = function ( self, key, value )
			error("Class forbid new attribute")
		end,
		__index = function (self, key)
			return class.create_factory[key]
		end
	})

return class
