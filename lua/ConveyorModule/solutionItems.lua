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

local projectTable = {}

local function createGroup(name, parent, uniquenessIndex)
    local group = { name = name, uuid = getUuid(name .. tostring(uniquenessIndex.idx)), parent = parent }
    uniquenessIndex.idx = uniquenessIndex.idx + 1
    return group
end

local function getOrCreateGroup(name, parent, uniquenessIndex)
    for _, v in ipairs(parent) do
        if v.name == name then
            return v
        end
    end

    local newGroup = createGroup(name, parent, uniquenessIndex)
    table.insert(parent, newGroup)
    return newGroup
end

local function walkPath(parentGroup, file, rootLength, uniquenessIndex)
    local leafPath = file:sub(rootLength)
    local targetGroup = parentGroup

    local folderEnd = leafPath:find("/")
    while folderEnd do
        local folder = leafPath:sub(1, folderEnd - 1)
        leafPath = leafPath:sub(folderEnd + 1)
        folderEnd = leafPath:find("/")

        targetGroup = getOrCreateGroup(folder, targetGroup, uniquenessIndex)
    end

    local files = targetGroup.files
    if files == nil then
        files = {}
        targetGroup.files = files
    end

    table.insert(files, file)
end

premake.override(premake.vstudio.sln2005, "projects", function(base, wks)
    local items = {}
    for _,item in ipairs(wks.solutionitems) do
        table.insert(items, { items = { item }, parent = projectTable })
    end

    local i, v = next(items, nil)
    local uniquenessIndex = { idx = 1 }
    while #items > 0 do
        local linkPrj = nil
        local otherCfg = nil
        table.remove(items, i)

        for name, files in pairs(v.items) do
            for a, pattern in pairs(files) do
                if type(pattern) == 'string' then
                    local groupWildcardStart = nil
                    if v.parent.name then
                        groupWildcardStart = v.parent.name:find("/[*]")
                    end

                    local wildcardStart = pattern:find("[*][*]")

                    if groupWildcardStart and wildcardStart then
                        for _,file in pairs(os.match(pattern)) do
                            if os.isfile(file) then
                                walkPath(v.parent, file, wildcardStart, uniquenessIndex)
                            end
                        end
                    else
                        local files = v.parent.files
                        if files == nil then
                            files = {}
                            v.parent.files = files
                        end

                        for _,file in pairs(os.match(pattern)) do
                            table.insert(files, file)
                        end
                    end
                elseif type(pattern) == 'table' then
                    local group = createGroup(a, v.parent, uniquenessIndex)
                    table.insert(items, { items = { pattern }, parent = group })
                    table.insert(v.parent, group)
                end
            end
        end

        i, v = next(items, nil)
    end

    --table.dump(projectTable)

    local projects = table.shallowCopy(projectTable)
    local i, prj = next(projects, nil)
    while #projects > 0 do
        table.remove(projects, i)
        for i = 1, #prj do
            table.insert(projects, prj[i])
        end

        local name = prj.name
        local wildcardStart = name:find("/[*]")
        if wildcardStart then
            name = name:sub(1, wildcardStart - 1)
        end

        premake.push('Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "'..name..'", "'.. name ..'", "{' .. prj.uuid .. '}"')
        premake.push("ProjectSection(SolutionItems) = preProject")
        if prj.files then
            for _, file in ipairs(prj.files) do
                file = path.rebase(file, ".", wks.location)
                premake.w(file.." = "..file)
            end
        end
        premake.pop("EndProjectSection")
        premake.pop("EndProject")
        i, prj = next(projects, nil)
    end

    base(wks)
end)

local function outputSolutionGroupings()
    local projects = table.shallowCopy(projectTable)
    local i, prj = next(projects, nil)
    while #projects > 0 do
        table.remove(projects, i)
        for i = 1, #prj do
            table.insert(projects, prj[i])
        end

        if prj.parent then
            p.w('{%s} = {%s}', prj.uuid, prj.parent.uuid)
        end
        i, prj = next(projects, nil)
    end
end


premake.override(premake.vstudio.sln2005, "nestedProjects", function(base, wks)
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

        outputSolutionGroupings()

        p.pop('EndGlobalSection')
    end
end)
