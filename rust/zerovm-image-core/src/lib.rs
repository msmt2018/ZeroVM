#![forbid(unsafe_code)]
use core::ops::Range;
#[derive(Debug,Clone,Copy,PartialEq,Eq)]
pub enum ParseError { OutOfBounds,IntegerOverflow,ResourceLimit,InvalidAlignment }
#[derive(Debug,Clone,Copy,PartialEq,Eq)]
pub struct ParseBudget { pub max_input_bytes:u64,pub max_items:u32,pub max_nesting_depth:u16 }
impl ParseBudget {
 pub fn check_input(&self,n:u64)->Result<(),ParseError>{if n>self.max_input_bytes{Err(ParseError::ResourceLimit)}else{Ok(())}}
 pub fn check_items(&self,n:u32)->Result<(),ParseError>{if n>self.max_items{Err(ParseError::ResourceLimit)}else{Ok(())}}
}
pub fn checked_range(offset:u64,length:u64,total:u64)->Result<Range<usize>,ParseError>{
 let end=offset.checked_add(length).ok_or(ParseError::IntegerOverflow)?;
 if end>total{return Err(ParseError::OutOfBounds)}
 Ok(usize::try_from(offset).map_err(|_|ParseError::OutOfBounds)?..usize::try_from(end).map_err(|_|ParseError::OutOfBounds)?)
}
pub fn checked_align_up(value:u64,alignment:u64)->Result<u64,ParseError>{
 if alignment==0||!alignment.is_power_of_two(){return Err(ParseError::InvalidAlignment)}
 value.checked_add(alignment-1).map(|v|v&!(alignment-1)).ok_or(ParseError::IntegerOverflow)
}
#[cfg(test)] mod tests{
 use super::*;
 #[test] fn bounded_range(){assert_eq!(checked_range(4,8,12),Ok(4..12));}
 #[test] fn overflow(){assert_eq!(checked_range(u64::MAX,2,u64::MAX),Err(ParseError::IntegerOverflow));}
 #[test] fn out_of_bounds(){assert_eq!(checked_range(8,8,12),Err(ParseError::OutOfBounds));}
 #[test] fn alignment(){assert_eq!(checked_align_up(9,8),Ok(16));assert_eq!(checked_align_up(9,3),Err(ParseError::InvalidAlignment));}
 #[test] fn budget(){let b=ParseBudget{max_input_bytes:1024,max_items:64,max_nesting_depth:8};assert_eq!(b.check_input(1025),Err(ParseError::ResourceLimit));}
}
