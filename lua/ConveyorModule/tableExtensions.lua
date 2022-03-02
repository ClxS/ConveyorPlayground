function table.shallowCopy(t)
    local t2 = {}
    for k,v in pairs(t) do
        t2[k] = v
    end
    return t2
end

function table.concatenate(t1,t2)
    for i=1,#t2 do
        t1[#t1+1] = t2[i]
    end
    return t1
end

function table.dump(t, indent)

  indent = indent or 0;

  local keys = {};

  for k in pairs(t) do
    keys[#keys+1] = k;
    table.sort(keys, function(a, b)
      local ta, tb = type(a), type(b);
      if (ta ~= tb) then
        return ta < tb;
      else
        return a < b;
      end
    end);
  end

  print(string.rep('  ', indent)..'{');
  indent = indent + 1;
  for k, v in pairs(t) do

    local key = k;
    if (type(key) == 'string') then
      if not (string.match(key, '^[A-Za-z_][0-9A-Za-z_]*$')) then
        key = "['"..key.."']";
      end
    elseif (type(key) == 'number') then
      key = "["..key.."]";
    end

    if (type(v) == 'table') then
      if (next(v)) then
        printf("%s%s =", string.rep('  ', indent), tostring(key));
        table.dump(v, indent);
      else
        printf("%s%s = {},", string.rep('  ', indent), tostring(key));
      end
    elseif (type(v) == 'string') then
      printf("%s%s = %s,", string.rep('  ', indent), tostring(key), "'"..v.."'");
    else
      printf("%s%s = %s,", string.rep('  ', indent), tostring(key), tostring(v));
    end
  end
  indent = indent - 1;
  print(string.rep('  ', indent)..'}');
end
