from lark import Tree, Token
from parselib.utils import dprint
from parselib.transforms import TopDown


class CommaTransformation(TopDown):
    """
    The comma transformation is aimed to handle the case such as
        k = (i++, j++)
    in the code.
    This type of operator is not supported in Verilog and thus we need to break
    the code into sequence of code such as:
        i++
        k = j++
    Note: k = j++ will be handled by another pass that gets j and j++ correctly
    To handle this, we break a hBinop , [ lhs rhs ] node into two.
    lhs will be appended to the parent and rhs remains
    """
    def __init__(self):
        super().__init__()
        self.broken_down_ops = []  # stores the operations broken down by ,
        self.has_comma = False

    def hcomma(self, tree):
        self.__push_up(tree)
        self.has_comma = True
        lhs, rhs = tree.children
        self.broken_down_ops.append(lhs)
        return rhs

    def stmts(self, tree):
        # dprint(tree.pretty())
        new_children = []
        for ch in tree.children:
            self.broken_down_ops = []
            if isinstance(ch, Tree):
                ch_new = self.visit(ch)
            else:
                ch_new = ch
            new_children.extend(map(lambda x: Tree('stmt', [x]), self.broken_down_ops))
            new_children.append(ch_new)
        tree.children = new_children
        return tree