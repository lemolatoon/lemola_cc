#[repr(C)]
#[derive(Debug)]
enum NodeKind {
    ND_ADD = 0,
    ND_SUB = 1,
    ND_MUL = 2,
    ND_DIV = 3,
    ND_NUM = 4,
}

#[repr(C)]
pub struct Node<'a> {
    kind: NodeKind,
    lhs: &'a Node<'a>,
    rhs: &'a Node<'a>,
    value: isize,
}

use std::{fmt::Debug, str::Matches};
impl Debug for Node<'_> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        if matches!(self.kind, NodeKind::ND_NUM) {
            f.debug_struct("Node")
                .field("kind", &self.kind)
                .field("value", &self.value)
                .finish()
        } else {
            f.debug_struct("Node")
                .field("kind", &self.kind)
                .field("lhs", &self.lhs)
                .field("rhs", &self.rhs)
                .finish()
        }
    }
}

#[no_mangle]
pub extern "C" fn hello() {
    println!("hello!!!!! from Rust");
}

#[no_mangle]
pub extern "C" fn ast_print(node: &Node) {
    println!("======RUST START========");
    println!("{:?}", node);
    println!("======RUST END==========");
}
