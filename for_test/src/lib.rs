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
    ND_REST,      // %
    ND_ASSIGN,    // =
    ND_IF,        // if
    ND_WHILE,     // while
    ND_FOR,       // for
    ND_LVAR,      // Local Variable
    ND_NUM,       // Integer
    ND_RETURN,    // return
    ND_BLOCKSTMT, // { <stmt>* }
    ND_CALLFUNC,  // function call
    ND_FUNCDEF,   // definition of function
    ND_ADDR,      // & <unary>
    ND_DEREF,     // * <unary>
    ND_DECLARE,   // int x;
}

#[repr(C)]
pub struct Node<'a> {
    kind: NodeKind,
    lhs: &'a Node<'a>,
    rhs: &'a Node<'a>,

    condition: *const Node<'a>,
    initialization: *const Node<'a>,
    increment: *const Node<'a>,
    els: *const Node<'a>,
    then: &'a Node<'a>,

    next: *const Node<'a>,

    first_arg: *const Node<'a>,

    name: *const c_char,
    len: c_int,
    arg_count: c_int,

    value: c_int,
    offset: c_int,
    type_: &'a Type<'a>,
}

impl<'a> Node<'a> {
    fn cond(&self) -> Option<&'a Node<'a>> {
        if !self.condition.is_null() {
            unsafe { self.condition.as_ref() }
        } else {
            None
        }
    }

    fn init(&self) -> Option<&'a Node<'a>> {
        if !self.initialization.is_null() {
            unsafe { self.initialization.as_ref() }
        } else {
            None
        }
    }

    fn els(&self) -> Option<&'a Node<'a>> {
        if !self.els.is_null() {
            unsafe { self.els.as_ref() }
        } else {
            None
        }
    }

    fn inc(&self) -> Option<&'a Node<'a>> {
        if !self.increment.is_null() {
            unsafe { self.increment.as_ref() }
        } else {
            None
        }
    }

    fn next(&self) -> Option<&'a Node<'a>> {
        assert!(!(self as *const Node).is_null());
        // if !(self as *const Node).is_null() {
        //     return None;
        // }
        return unsafe { self.next.as_ref() };
        // if !self.next.is_null() {
        //     unsafe { self.next.as_ref() }
        // } else {
        //     None
        // }
    }
}

use std::{
    arch::asm,
    ffi::CStr,
    fmt::{Debug, DebugStruct, Formatter},
    os::raw::{c_char, c_int},
};
impl Debug for Node<'_> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        use NodeKind::*;
        // debug_struct_next!(self, f, &self.next())
        assert!(!(self as *const Node).is_null());
        if let Some(next) = self.next() {
            match self.kind {
                ND_NUM => f
                    .debug_struct("Node")
                    .field("kind", &self.kind)
                    .field("value", &self.value)
                    .field("type", &self.type_)
                    .field("next", next)
                    .finish(),
                ND_LVAR => f
                    .debug_struct("Node")
                    .field("kind", &self.kind)
                    .field("offset", &self.offset)
                    .field("type", &self.type_)
                    .field("next", next)
                    .finish(),
                ND_RETURN => f
                    .debug_struct("Node")
                    .field("kind", &self.kind)
                    .field("lhs", &self.lhs)
                    .field("next", next)
                    .finish(),
                ND_IF => f
                    .debug_struct("Node")
                    .field("kind", &self.kind)
                    .field("cond", &self.cond())
                    .field("then", &self.then)
                    .field("else", &self.els())
                    .field("next", next)
                    .finish(),
                ND_WHILE => f
                    .debug_struct("Node")
                    .field("kind", &self.kind)
                    .field("cond", &self.cond())
                    .field("then", &self.then)
                    .field("next", next)
                    .finish(),
                ND_FOR => f
                    .debug_struct("Node")
                    .field("kind", &self.kind)
                    .field("init", &self.init())
                    .field("cond", &self.cond())
                    .field("inc", &self.inc())
                    .field("then", &self.then)
                    .field("next", next)
                    .finish(),
                ND_BLOCKSTMT => f
                    .debug_struct("Node")
                    .field("kind", &self.kind)
                    .field("next", next)
                    .finish(),
                ND_CALLFUNC => f
                    .debug_struct("Node")
                    .field("kind", &self.kind)
                    .field(
                        "name",
                        &&unsafe { CStr::from_ptr(self.name) }
                            .to_str()
                            .unwrap()
                            .chars()
                            .zip(0..)
                            .filter(|(_, i)| i < &self.len)
                            .map(|(c, _)| c)
                            .collect::<String>(),
                    )
                    .field("arg_count", &self.arg_count)
                    .field("first_arg", unsafe { &self.first_arg.as_ref() })
                    .finish(),
                ND_FUNCDEF => f
                    .debug_struct("Node")
                    .field("kind", &self.kind)
                    .field(
                        "name",
                        &&unsafe { CStr::from_ptr(self.name) }
                            .to_str()
                            .unwrap()
                            .chars()
                            .zip(0..)
                            .filter(|(_, i)| i < &self.len)
                            .map(|(c, _)| c)
                            .collect::<String>(),
                    )
                    .field("arg_count", &self.arg_count)
                    .field("first_arg", unsafe { &self.first_arg.as_ref() })
                    .field("then", &self.then)
                    .finish(),
                ND_ADDR | ND_DEREF => f
                    .debug_struct("Node")
                    .field("kind", &self.kind)
                    .field("lhs", &self.lhs)
                    .field("type", &self.type_.get())
                    .field("next", next)
                    .finish(),
                ND_DECLARE => f
                    .debug_struct("Node")
                    .field("kind", &self.kind)
                    .field("lhs", &self.lhs)
                    .field("rhs", unsafe { &(self.rhs as *const Node).as_ref() })
                    .field("next", next)
                    .finish(),
                _ => f
                    .debug_struct("Node")
                    .field("kind", &self.kind)
                    .field("lhs", &self.lhs)
                    .field("rhs", &self.rhs)
                    .field("type", &self.type_.get())
                    .field("next", next)
                    .finish(),
            }
        } else {
            match self.kind {
                ND_NUM => f
                    .debug_struct("Node")
                    .field("kind", &self.kind)
                    .field("value", &self.value)
                    .field("type", &self.type_)
                    .finish(),
                ND_LVAR => f
                    .debug_struct("Node")
                    .field("kind", &self.kind)
                    .field("offset", &self.offset)
                    .field("type", &self.type_)
                    .finish(),
                ND_RETURN => f
                    .debug_struct("Node")
                    .field("kind", &self.kind)
                    .field("lhs", &self.lhs)
                    .finish(),
                ND_IF => f
                    .debug_struct("Node")
                    .field("kind", &self.kind)
                    .field("cond", &self.cond())
                    .field("then", &self.then)
                    .field("else", &self.els())
                    .finish(),
                ND_WHILE => f
                    .debug_struct("Node")
                    .field("kind", &self.kind)
                    .field("cond", &self.cond())
                    .field("then", &self.then)
                    .finish(),
                ND_FOR => f
                    .debug_struct("Node")
                    .field("kind", &self.kind)
                    .field("init", &self.init())
                    .field("cond", &self.cond())
                    .field("inc", &self.inc())
                    .field("then", &self.then)
                    .finish(),
                ND_BLOCKSTMT => f.debug_struct("Node").field("kind", &self.kind).finish(),
                ND_CALLFUNC => f
                    .debug_struct("Node")
                    .field("kind", &self.kind)
                    .field(
                        "name",
                        &&unsafe { CStr::from_ptr(self.name) }
                            .to_str()
                            .unwrap()
                            .chars()
                            .zip(0..)
                            .filter(|(_, i)| i < &self.len)
                            .map(|(c, _)| c)
                            .collect::<String>(),
                    )
                    .field("arg_count", &self.arg_count)
                    .field("first_arg", unsafe { &self.first_arg.as_ref() })
                    .finish(),
                ND_FUNCDEF => f
                    .debug_struct("Node")
                    .field("kind", &self.kind)
                    .field(
                        "name",
                        &&unsafe { CStr::from_ptr(self.name) }
                            .to_str()
                            .unwrap()
                            .chars()
                            .zip(0..)
                            .filter(|(_, i)| i < &self.len)
                            .map(|(c, _)| c)
                            .collect::<String>(),
                    )
                    .field("arg_count", &self.arg_count)
                    .field("first_arg", unsafe { &self.first_arg.as_ref() })
                    .field("then", &self.then)
                    .finish(),
                ND_ADDR | ND_DEREF => f
                    .debug_struct("Node")
                    .field("kind", &self.kind)
                    .field("type", &self.type_.get())
                    .field("lhs", &self.lhs)
                    .finish(),
                ND_DECLARE => f
                    .debug_struct("Node")
                    .field("kind", &self.kind)
                    .field("lhs", &self.lhs)
                    .field("rhs", unsafe { &(self.rhs as *const Node).as_ref() })
                    .finish(),
                _ => f
                    .debug_struct("Node")
                    .field("kind", &self.kind)
                    .field("lhs", &self.lhs)
                    .field("rhs", &self.rhs)
                    .field("type", &self.type_.get())
                    .finish(),
            }
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
    TK_RESERVED, // operator
    TK_IDENT,    // identifier
    TK_RETURN,   // return
    TK_IF,       // if
    TK_WHILE,    // while
    TK_FOR,      // for
    TK_ELSE,     // else
    TK_NUM,      // number literal
    TK_INT,      // int
    TK_SIZEOF,   // sizeof
    TK_EOF,      // End of File
}

#[repr(C)]
pub struct Token<'a> {
    kind: TokenKind,
    next: &'a Token<'a>,
    value: c_int,
    str: *const c_char,
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
                    &&unsafe { CStr::from_ptr(self.str) }
                        .to_str()
                        .unwrap()
                        .chars()
                        .zip(0..)
                        .filter(|(_, i)| *i < self.len)
                        .map(|(c, _)| c)
                        .collect::<String>(),
                )
                .finish(),
            _ => f
                .debug_struct("Token")
                .field("kind", &self.kind)
                .field(
                    "str",
                    &&unsafe { CStr::from_ptr(self.str) }
                        .to_str()
                        .unwrap()
                        .chars()
                        .zip(0..)
                        .filter(|(_, i)| *i < self.len)
                        .map(|(c, _)| c)
                        .collect::<String>(),
                )
                .finish(),
        }
    }
}

#[no_mangle]
pub extern "C" fn token_print(token: &Token) {
    println!("{:?}", token);
}

#[repr(C)]
pub struct LVar<'a> {
    next: *const LVar<'a>,
    name: &'a c_char,
    len: c_int,
    offset: c_int,
    type_: &'a Type<'a>,
}

impl Debug for LVar<'_> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        if let Some(next) = unsafe { self.next.as_ref() } {
            f.debug_struct("LVar")
                .field("next", next)
                .field("name", &self.name)
                .field("len", &self.len)
                .field("offset", &self.offset)
                .field("type", &self.type_)
                .finish()
        } else {
            f.debug_struct("LVar is NULL").finish()
        }
    }
}

#[derive(Debug)]
#[repr(C)]
enum TypeKind {
    NONE,
    INT,
    PTR,
    ARRAY,
}

#[repr(C)]
pub struct Type<'a> {
    ty: TypeKind,
    ptr_to: &'a Type<'a>,
    array_size: usize,
}

impl<'a> Debug for Type<'a> {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        assert!(!(self as *const Type<'_>).is_null());
        match self.ty {
            TypeKind::NONE => f.debug_struct("Type").field("ty", &self.ty).finish(),
            TypeKind::INT => f.debug_struct("Type").field("ty", &self.ty).finish(),
            TypeKind::PTR => f
                .debug_struct("Type")
                .field("ty", &self.ty)
                .field("ptr_to", &self.ptr_to)
                .finish(),
            TypeKind::ARRAY => f
                .debug_struct("Type")
                .field("ty", &self.ty)
                .field("ptr_to", &self.ptr_to)
                .field("array_size", &self.array_size)
                .finish(),
        }
    }
}

impl Type<'_> {
    fn get(&self) -> Option<&Self> {
        if (self as *const Type<'_>).is_null() {
            return None;
        } else {
            return unsafe { (self as *const Type<'_>).as_ref() };
        }
    }
}

#[no_mangle]
pub extern "C" fn type_print(type_: &Type<'_>) {
    // assert!(!type_.is_null());
    // let type_: &Type<'_> = unsafe { type_.as_ref().unwrap() };
    println!("{:?}", type_);
}

#[no_mangle]
pub extern "C" fn lvar_print(lvar: &LVar<'_>) {
    println!("RUSTSTART!!!!!!!!!!!!!!!!!!");
    println!("{:?}", lvar);
}
