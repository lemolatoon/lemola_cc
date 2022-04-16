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

use std::{
    fmt::Debug,
    os::raw::c_char,
    ptr::{slice_from_raw_parts, slice_from_raw_parts_mut},
};
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

// token

#[repr(C)]
#[derive(Debug)]
enum TokenKind {
    TK_RESERVED,
    TK_NUM,
    TK_EOF,
}

#[repr(C)]
pub struct Token<'a> {
    kind: TokenKind,
    next: &'a Token<'a>,
    value: isize,
    str: &'a c_char,
    len: isize,
}

impl Debug for Token<'_> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        // let str = unsafe {
        //     slice_from_raw_parts_mut::<u8>(self.str, self.len as usize)
        //         .as_ref()
        //         .unwrap()
        // }
        // .into_iter()
        // .map(|c| char::from_u32(*c as u32).unwrap())
        // .collect::<String>();
        if matches!(self.kind, TokenKind::TK_EOF) {
            f.debug_struct("EOF").finish()
        } else {
            f.debug_struct("Token")
                .field("kind", &self.kind)
                // .field("next", &self.next)
                .field("value", &self.value)
                .field(
                    "str",
                    &char::from_u32(self.str.to_string().parse().unwrap()).unwrap(),
                )
                .field("len", &self.len)
                .finish()
        }
    }
}

#[no_mangle]
pub extern "C" fn token_print(token: &Token) {
    println!("{:?}", token);
}
