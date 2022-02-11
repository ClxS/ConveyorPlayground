-- From: https://github.com/1bardesign/batteries
--[[
Copyright 2021 Max Cahill

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
]]

-- Alterations:
--   Moved definitions out of the stringx table into the global table


--split a string on a delimiter into an ordered table
function split(self, delim, limit)
	delim = delim or ""
	limit = (limit ~= nil and limit) or math.huge

	if limit then
		assert(limit >= 0, "max_split must be positive!")
	end

	--we try to create as little garbage as possible!
	--only one table to contain the result, plus the split strings.
	--so we do two passes, and  work with the bytes underlying the string
	--partly because string.find is not compiled on older luajit :)
	local res = {}
	local length = self:len()
	--
	local delim_length = delim:len()
	--empty delim? split to individual characters
	if delim_length == 0 then
		for i = 1, length do
			table.insert(res, self:sub(i, i))
		end
		return res
	end
	local delim_start = delim:byte(1)
	--pass 1
	--collect split sites
	local i = 1
	while i <= length do
		--scan for delimiter
		if self:byte(i) == delim_start then
			local has_whole_delim = true
			for j = 2, delim_length do
				if self:byte(i + j - 1) ~= delim:byte(j) then
					has_whole_delim = false
					break
				end
			end
			if has_whole_delim then
				if #res < limit then
					table.insert(res, i)
				else
					break
				end
			end
			--iterate forward
			i = i + delim_length
		else
			--iterate forward
			i = i + 1
		end
	end
	--pass 2
	--collect substrings
	i = 1
	for si, j in ipairs(res) do
		res[si] = self:sub(i, j-1)
		i = j + delim_length
	end
	--add the final section
	table.insert(res, self:sub(i, -1))
	--return the collection
	return res
end

function deindent(s, keep_trailing_empty)
	--detect windows or unix newlines
	local windows_newlines = s:find("\r\n", nil, true)
	local newline = windows_newlines and "\r\n" or "\n"
	--split along newlines
	local lines = split(s, newline)
	--detect and strip any leading blank lines
	local leading_newline = false
	while lines[1] == "" do
		leading_newline = true
		table.remove(lines, 1)
	end

	--nothing to do
	if #lines == 0 then
		return ""
	end

	--detect indent
	local _, _, indent = lines[1]:find("^([ \t]*)")
	local indent_len = indent and indent:len() or 0

	--not indented
	if indent_len == 0 then
		return table.concat(lines, newline)
	end

	--de-indent the lines
	local res = {}
	for _, line in ipairs(lines) do
		local line_start = line:sub(1, indent:len())
		local start_len = line_start:len()
		if
			line_start == indent
			or (
				start_len < indent_len
				and line_start == indent:sub(1, start_len)
			)
		then
			line = line:sub(start_len + 1)
		end
		table.insert(res, line)
	end

	--should
	if not keep_trailing_empty then
		if res[#res] == "" then
			table.remove(res)
		end
	end

	return table.concat(res, newline)
end
