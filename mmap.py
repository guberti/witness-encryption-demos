class Group:
    def __init__(self, P, id):
        self.P = P
        assert id > 0, "id must be greater than 0"
        self.id = id

    def __str__(self):
        return f"Group(id={self.id}, P={self.P})"

    def __eq__(self, other):
        return self.P == other.P and self.id == other.id

    def get_generator(self):
        return Element(1, self)
        
class Element:
    def __init__(self, value, group: Group):
        self.group = group
        self.value = value # generator power

    def __str__(self):
        return f"Element(value={self.value}, group={self.group})"
    
    def __mul__(self, other):
        assert self.group.id == other.group.id, "id does not match"
        return Element((self.value + other.value) % self.group.P, self.group)

    def __eq__ (self, other):
        return self.value == other.value and self.group == other.group
    
    def __pow__(self, n):
        assert isinstance(n, int), "Only integer exponents"
        return Element((self.value * n) % self.group.P, self.group)

class MMap:
    def __init__(self, n, P):
        self.n = n
        self.P = P
        self.groups = [Group(P, i) for i in range(1, n + 1)]
        self.generators = [group.get_generator() for group in self.groups]

    def __str__(self):
        return f"MMap(n={self.n}, P={self.P}, groups = [{', '.join(map(str, self.groups))}], generators = {', '.join(map(str, self.generators))})"

    def get_group(self, id):
        # will return the group with id=id
        return self.groups[id - 1]

    def get_generator(self, id):
        return self.generators[id - 1]

    def bilinear_map(self, e: Element, f: Element):
        new_gp_id = e.group.id + f.group.id
        assert new_gp_id <= self.n, f"{new_gp_id}, {self.n}"
        return Element((e.value * f.value) % self.P, self.get_group(new_gp_id))
        
