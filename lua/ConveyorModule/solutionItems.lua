require('vstudio')

local p = premake
local vstudio = p.vstudio
local project = p.project
local config = p.config
local tree = p.tree

premake.api.register {
  name = "solutionitems",
  scope = "workspace",
  kind = "list:table",
}

local function getUuid(value)
    return os.uuid("Solution Items:" .. value)
end

premake.override(premake.vstudio.sln2005, "projects", function(base, wks)

    local items = table.shallowCopy(wks.solutionitems)

    local i, v = next(items, nil)
    while #items > 0 do
        local linkPrj = nil
        local otherCfg = nil
        table.remove(items, i)
        for name, files in pairs(v) do
            premake.push('Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "'..name..'", "'..name..'", "{' .. getUuid(name) .. '}"')
            premake.push("ProjectSection(SolutionItems) = preProject")
            for a, pattern in pairs(files) do
                if type(pattern) == 'string' then
                    for _,file in pairs(os.match(pattern)) do
                        file = path.rebase(file, ".", wks.location)
                        premake.w(file.." = "..file)
                    end
                elseif type(pattern) == 'table' then
                    table.insert(items, { [a] = pattern })
                end
            end
            premake.pop("EndProjectSection")
            premake.pop("EndProject")
        end

        i, v = next(items, nil)
    end

    base(wks)
end)


premake.override(premake.vstudio.sln2005, "nestedProjects", function(base, wks)
    local solutionItemGrouping = {}
    local items = table.shallowCopy(wks.solutionitems)
    local i, v = next(items, nil)
    while #items > 0 do
        local linkPrj = nil
        local otherCfg = nil
        table.remove(items, i)
        for name, files in pairs(v) do
            for a, pattern in pairs(files) do
                if type(pattern) == 'table' then
                    table.insert(items, { [a] = pattern })
                    solutionItemGrouping[getUuid(a)] = getUuid(name)
                end
            end
        end

        i, v = next(items, nil)
    end

    local tr = p.workspace.grouptree(wks)
    if tree.hasbranches(tr) then
        p.push('GlobalSection(NestedProjects) = preSolution')
        tree.traverse(tr, {
            onnode = function(n)
                if n.parent.uuid then
                    p.w('{%s} = {%s}', (n.project or n).uuid, n.parent.uuid)
                end
            end
        })

        for k,v in pairs(solutionItemGrouping) do
            p.w('{%s} = {%s}', k, v)
        end

        p.pop('EndGlobalSection')
    end
end)
