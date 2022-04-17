#[allow(dead_code)]
#[allow(non_camel_case_types)]
#[repr(C)]
#[derive(Debug)]
enum NodeKind {
    ND_EQ,        // ==
    ND_NEQ,       // !=
    ND_SMALLER,   // <
    ND_SMALLEREQ, // <=
    ND_ADD,       // +
    ND_SUB,       // -
    ND_MUL,       // *
    ND_DIV,       // /
    ND_ASSIGN,    // =
    ND_LVAR,      // Local Variable
    ND_NUM,       // Integer
}

#[repr(C)]
pub struct Node<'a> {
    kind: NodeKind,
    lhs: &'a Node<'a>,
    rhs: &'a Node<'a>,
    value: c_int,
    offset: c_int,
}

use std::{
    fmt::Debug,
    os::raw::{c_char, c_int},
};
impl Debug for Node<'_> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        use NodeKind::*;
        match self.kind {
            ND_NUM => f
                .debug_struct("Node")
                .field("kind", &self.kind)
                .field("value", &self.value)
                .finish(),
            ND_LVAR => f
                .debug_struct("Node")
                .field("kind", &self.kind)
                .field("offset", &self.offset)
                .finish(),
            _ => f
                .debug_struct("Node")
                .field("kind", &self.kind)
                .field("lhs", &self.lhs)
                .field("rhs", &self.rhs)
                .finish(),
        }
    }
}

#[no_mangle]
pub extern "C" fn hello() {
    println!("hello!!!!! from Rust");
}

#[no_mangle]
pub extern "C" fn ast_print(node: &Node) {
    println!("{:?}", node);
}

// token

#[allow(dead_code)]
#[allow(non_camel_case_types)]
#[repr(C)]
#[derive(Debug)]
enum TokenKind {
    TK_RESERVED,
    TK_IDNET,
    TK_NUM,
    TK_EOF,
}

#[repr(C)]
pub struct Token<'a> {
    kind: TokenKind,
    next: &'a Token<'a>,
    value: c_int,
    str: &'a c_char,
    len: c_int,
}

impl Debug for Token<'_> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        use TokenKind::*;
        match self.kind {
            TK_EOF => f.debug_struct("EOF").finish(),
            TK_NUM => f
                .debug_struct("Token")
                .field("kind", &self.kind)
                .field("value", &self.value)
                .field(
                    "str",
                    &char::from_u32(self.str.to_string().parse().unwrap()).unwrap(),
                )
                .field("len", &self.len)
                .finish(),
            _ => f
                .debug_struct("Token")
                .field("kind", &self.kind)
                .field(
                    "str",
                    &char::from_u32(self.str.to_string().parse().unwrap()).unwrap(),
                )
                .field("len", &self.len)
                .finish(),
        }
    }
}

#[no_mangle]
pub extern "C" fn token_print(token: &Token) {
    println!("{:?}", token);
}
