/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: vvm_func.cc,v 1.17 2001/07/25 03:10:50 steve Exp $"
#endif

# include "config.h"

# include  "vvm_func.h"

vpip_bit_t vvm_unop_and(const vvm_bitset_t&r)
{
      vpip_bit_t v = r[0];

      for (unsigned idx = 1 ;  idx < r.get_width() ;  idx += 1)
	    v = B_AND(v, r[idx]);

      return v;
}

vpip_bit_t vvm_unop_nand(const vvm_bitset_t&r)
{
      vpip_bit_t v = vvm_unop_and(r);
      return B_NOT(v);
}

vpip_bit_t vvm_unop_lnot(const vvm_bitset_t&r)
{
      vpip_bit_t v = vvm_unop_or(r);
      return B_NOT(v);
}

void vvm_unop_not(vvm_bitset_t&v, const vvm_bitset_t&p)
{
      assert(v.nbits <= p.nbits);
      for (unsigned idx = 0 ;  idx < v.nbits ;  idx += 1)
	    v[idx] = B_NOT(p[idx]);
}

vpip_bit_t vvm_unop_or(const vvm_bitset_t&r)
{
      for (unsigned idx = 0 ;  idx < r.get_width() ;  idx += 1) {
	    if (B_IS1(r.get_bit(idx)))
		  return St1;
      }

      return St0;
}

vpip_bit_t vvm_unop_nor(const vvm_bitset_t&r)
{
      vpip_bit_t v = vvm_unop_or(r);
      return B_NOT(v);
}

void vvm_unop_uminus(vvm_bitset_t&v, const vvm_bitset_t&l)
{
      vvm_unop_not(v, l);
      vpip_bit_t carry = St1;
      for (unsigned i = 0 ;  i < v.nbits ;  i += 1)
	    v[i] = add_with_carry(v[i], St0, carry);

}

vpip_bit_t vvm_unop_xor(const vvm_bitset_t&r)
{
      vpip_bit_t v = St0;

      for (unsigned idx = 0 ;  idx < r.get_width() ;  idx += 1) {
	    if (B_IS1(r.get_bit(idx)))
		  v = B_NOT(v);
      }
      return v;
}

vpip_bit_t vvm_unop_xnor(const vvm_bitset_t&r)
{
      vpip_bit_t v = vvm_unop_xor(r);
      return B_NOT(v);
}

/*
 * Do a bitwise AND into the result. We only need to calculate enough
 * bits to fill the result. If I need to extend either value, extend
 * it with St0.
 */
void vvm_binop_and(vvm_bitset_t&v, const vvm_bitset_t&l, const vvm_bitset_t&r)
{
      unsigned min = v.nbits;
      if (r.nbits < min) min = r.nbits;
      if (l.nbits < min) min = l.nbits;

      for (unsigned idx = 0 ;  idx < min ;  idx += 1)
	    v[idx] = B_AND(l[idx], r[idx]);

      for (unsigned idx = min ;  idx < v.nbits ;  idx += 1)
	    v[idx] = St0;
}

void vvm_binop_minus(vvm_bitset_t&v, const vvm_bitset_t&l,
		     const vvm_bitset_t&r)
{
      vvm_unop_not(v, r);
      vpip_bit_t carry = St1;
      for (unsigned idx = 0 ;  idx < v.nbits ;  idx += 1)
	    v[idx] = add_with_carry(l[idx], v[idx], carry);
}

void vvm_binop_nor(vvm_bitset_t&v, const vvm_bitset_t&l, const vvm_bitset_t&r)
{
      unsigned min = v.nbits;
      if (r.nbits < min) min = r.nbits;
      if (l.nbits < min) min = l.nbits;

      for (unsigned idx = 0 ;  idx < min ;  idx += 1)
	    v[idx] = B_NOT(B_OR(l[idx], r[idx]));

      for (unsigned idx = min ;  idx < v.nbits ;  idx += 1)
	    v[idx] = B_NOT(r.nbits > min? r[idx] : l[idx]);
}

void vvm_binop_or(vvm_bitset_t&v, const vvm_bitset_t&l, const vvm_bitset_t&r)
{
      unsigned min = v.nbits;
      if (r.nbits < min) min = r.nbits;
      if (l.nbits < min) min = l.nbits;

      for (unsigned idx = 0 ;  idx < min ;  idx += 1)
	    v[idx] = B_OR(l[idx], r[idx]);

      for (unsigned idx = min ;  idx < v.nbits ;  idx += 1)
	    v[idx] = r.nbits > min? r[idx] : l[idx];
}

/*
 * This function adds two vectors to make a result vector. The
 * operands and the result have their sizes already determined, so we
 * take those into account.
 */
void vvm_binop_plus(vvm_bitset_t&v, const vvm_bitset_t&l, const vvm_bitset_t&r)
{
      vpip_bit_t carry = St0;

      unsigned top = v.nbits;
      if (l.nbits < top)
	    top = l.nbits;
      if (r.nbits < top)
	    top = r.nbits;

      unsigned idx;

	/* First do the addition for the part of the vector where we
	   know that the bits from both inputs are present. */
      for (idx = 0 ;  idx < top ;  idx += 1)
	    v[idx] = add_with_carry(l[idx], r[idx], carry);

	/* Now continue the addition, padding the shorter vector with
	   St0 until we fill the result vector. */
      for (  ;  idx < v.nbits ;  idx += 1) {
	    vpip_bit_t lv = (idx < l.nbits) ? l[idx] : St0;
	    vpip_bit_t rv = (idx < r.nbits) ? r[idx] : St0;
	    v[idx] = add_with_carry(lv, rv, carry);
      }
}

void vvm_binop_shiftl(vvm_bitset_t&v,
		      const vvm_bitset_t&l,
		      const vvm_bitset_t&r)
{
      assert(v.nbits <= l.nbits);
      vvm_u32 s = r.as_unsigned();
      for (unsigned idx = 0 ;  idx < v.nbits; idx += 1)
	    v[idx] = (idx < s) ? St0 : l[idx-s];
}

void vvm_binop_shiftr(vvm_bitset_t&v,
		      const vvm_bitset_t&l,
		      const vvm_bitset_t&r)
{
      vvm_u32 s = r.as_unsigned();

      for (unsigned idx = 0 ;  idx < v.nbits ; idx += 1)
	    v[idx] = ((idx+s) < l.nbits) ? l[idx+s] : St0;
}

void vvm_binop_xnor(vvm_bitset_t&v, const vvm_bitset_t&l, const vvm_bitset_t&r)
{
      assert(v.nbits <= l.nbits);
      assert(v.nbits <= r.nbits);
      for (unsigned idx = 0 ;  idx < v.nbits ;  idx += 1)
	    v[idx] = B_NOT(B_XOR(l[idx], r[idx]));
}

void vvm_binop_xor(vvm_bitset_t&v, const vvm_bitset_t&l, const vvm_bitset_t&r)
{
      assert(v.nbits <= l.nbits);
      assert(v.nbits <= r.nbits);
      for (unsigned idx = 0 ;  idx < v.nbits ;  idx += 1)
	    v[idx] = B_XOR(l[idx], r[idx]);
}

vpip_bit_t vvm_binop_eq(const vvm_bitset_t&l, const vvm_bitset_t&r)
{
      const unsigned lwid = l.get_width();
      const unsigned rwid = r.get_width();

      if (lwid <= rwid) {
	    for (unsigned idx = 0 ;  idx < lwid ;  idx += 1) {
		  if (B_ISXZ(l.get_bit(idx)))
			return StX;

		  if (B_ISXZ(r.get_bit(idx)))
			return StX;

		  if (! B_EQ(l.get_bit(idx), r.get_bit(idx)))
			return St0;

	    }

	    for (unsigned idx = lwid ;  idx < rwid ;  idx += 1) {

		  if (B_IS0(r.get_bit(idx)))
			continue;

		  if (B_IS1(r.get_bit(idx)))
			return St0;

		  return StX;
	    }
		  
	    return St1;

      } else {
	    for (unsigned idx = 0 ;  idx < rwid ;  idx += 1) {
		  if (B_ISXZ(l.get_bit(idx)))
			return StX;

		  if (B_ISXZ(r.get_bit(idx)))
			return StX;

		  if (! B_EQ(l.get_bit(idx), r.get_bit(idx)))
			return St0;

	    }
	    for (unsigned idx = rwid ;  idx < lwid ;  idx += 1) {

		  if (B_IS0(l.get_bit(idx)))
			continue;

		  if (B_IS1(l.get_bit(idx)))
			return St0;

		  return StX;
	    }
		  
	    return St1;
      }
}

vpip_bit_t vvm_binop_ne(const vvm_bitset_t&l, const vvm_bitset_t&r)
{
      vpip_bit_t result = vvm_binop_eq(l,r);
      return B_NOT(result);
}

vpip_bit_t vvm_binop_eeq(const vvm_bitset_t&l,  const vvm_bitset_t&r)
{
      const unsigned lwid = l.get_width();
      const unsigned rwid = r.get_width();

      if (lwid <= rwid) {
	    for (unsigned idx = 0 ;  idx < lwid ;  idx += 1)
		  if (! B_EQ(l.get_bit(idx), r.get_bit(idx)))
			return St0;

	    for (unsigned idx = lwid ;  idx < rwid ;  idx += 1)
		  if (! B_IS0(r.get_bit(idx)))
			return St0;
		  
      } else {
	    for (unsigned idx = 0 ;  idx < rwid ;  idx += 1)
		  if (! B_EQ(l.get_bit(idx), r.get_bit(idx)))
			return St0;

	    for (unsigned idx = rwid ;  idx < lwid ;  idx += 1)
		  if (! B_IS0(l.get_bit(idx)))
			return St0;
		  
      }

      return St1;
}

vpip_bit_t vvm_binop_nee(const vvm_bitset_t&l, const vvm_bitset_t&r)
{
      vpip_bit_t result = vvm_binop_eeq(l,r);
      return B_NOT(result);
}

vpip_bit_t vvm_binop_xeq(const vvm_bitset_t&l, const vvm_bitset_t&r)
{
      const unsigned lwid = l.get_width();
      const unsigned rwid = r.get_width();

      if (lwid <= rwid) {
	    for (unsigned idx = 0 ;  idx < lwid ;  idx += 1) {
		  if (B_ISXZ(l.get_bit(idx)))
			continue;
		  if (B_ISXZ(r.get_bit(idx)))
			continue;
		  if (! B_EQ(l.get_bit(idx), r.get_bit(idx)))
			return St0;
	    }

	    for (unsigned idx = lwid ;  idx < rwid ;  idx += 1) {
		  if (B_ISXZ(r.get_bit(idx)))
			continue;
		  if (! B_IS0(r.get_bit(idx)))
			return St0;
	    }
		  
      } else {
	    for (unsigned idx = 0 ;  idx < rwid ;  idx += 1) {
		  if (B_ISXZ(l.get_bit(idx)))
			continue;
		  if (B_ISXZ(r.get_bit(idx)))
			continue;
		  if (! B_EQ(l.get_bit(idx), r.get_bit(idx)))
			return St0;

	    }
	    for (unsigned idx = rwid ;  idx < lwid ;  idx += 1) {
		  if (B_ISXZ(l.get_bit(idx)))
			continue;
		  if (! B_IS0(l.get_bit(idx)))
			return St0;
	    }
      }

      return St1;
}

vpip_bit_t vvm_binop_zeq(const vvm_bitset_t&l, const vvm_bitset_t&r)
{
      const unsigned lwid = l.get_width();
      const unsigned rwid = r.get_width();

      if (lwid <= rwid) {
	    for (unsigned idx = 0 ;  idx < lwid ;  idx += 1) {
		  if (B_ISZ(l.get_bit(idx)) || B_ISZ(r.get_bit(idx)))
			continue;
		  if (! B_EQ(l.get_bit(idx), r.get_bit(idx)))
			return St0;
	    }

	    for (unsigned idx = lwid ;  idx < rwid ;  idx += 1) {
		  if (B_ISZ(r.get_bit(idx)))
			continue;
		  if (! B_IS0(r.get_bit(idx)))
			return St0;

	    }
		  
      } else {
	    for (unsigned idx = 0 ;  idx < rwid ;  idx += 1) {
		  if (B_ISZ(l.get_bit(idx)) || B_ISZ(r.get_bit(idx)))
			continue;
		  if (! B_EQ(l.get_bit(idx), r.get_bit(idx)))
			return St0;
	    }

	    for (unsigned idx = rwid ;  idx < lwid ;  idx += 1) {
		  if (B_ISZ(l.get_bit(idx)))
			continue;
		  if (! B_IS0(l.get_bit(idx)))
			return St0;
	    }
      }

      return St1;
}

vpip_bit_t vvm_binop_lt(const vvm_bitset_t&l, const vvm_bitset_t&r)
{
      vpip_bit_t result;
      result = St0;
      const unsigned lwid = l.get_width();
      const unsigned rwid = r.get_width();

      const unsigned common = (lwid < rwid)? lwid : rwid;

      for (unsigned idx = 0 ;  idx < common ;  idx += 1)
	    result = less_with_cascade(l.get_bit(idx), r.get_bit(idx), result);

      if (lwid > rwid) {
	    for (unsigned idx = rwid ;  idx < lwid ;  idx += 1)
		  result = less_with_cascade(l.get_bit(idx), St0, result);
      } else {
	    for (unsigned idx = lwid ;  idx < rwid ;  idx += 1)
		  result = less_with_cascade(St0, r.get_bit(idx), result);
      }

      return result;
}

/*
 * This is the < operator that applies to signed operands.
 */
vpip_bit_t vvm_binop_lt_s(const vvm_bitset_t&l, const vvm_bitset_t&r)
{
      vpip_bit_t l_pad = l.get_bit(l.get_width()-1);
      vpip_bit_t r_pad = r.get_bit(r.get_width()-1);

	/* If l>=0 and r>=0, return $unsigned(l) < $unsigned(r) */
      if (B_IS0(l_pad) && B_IS0(r_pad))
	    return vvm_binop_lt(l, r);

	/* If l < 0 and r < 0, return $unsigned(r) < $unsigned(l) */
      if (B_IS1(l_pad) && B_IS1(r_pad))
	    return vvm_binop_lt(r, l);

	/* If l >= 0 and r < 0, return false; */
      if (B_IS0(l_pad) && B_IS1(r_pad))
	    return St0;

	/* if l < 0 and r >= 0, return true; */
      if (B_IS1(l_pad) && B_IS0(r_pad))
	    return St1;

	/* Otherwise, one or the other side is unknown. Return X. */
      return StX;
}

vpip_bit_t vvm_binop_le(const vvm_bitset_t&l, const vvm_bitset_t&r)
{
      vpip_bit_t result = St1;
      const unsigned lwid = l.get_width();
      const unsigned rwid = r.get_width();
      const unsigned common = (lwid < rwid)? lwid : rwid;

      for (unsigned idx = 0 ;  idx < common ;  idx += 1)
	    result = less_with_cascade(l.get_bit(idx), r.get_bit(idx), result);

      if (lwid > rwid) {
	    for (unsigned idx = rwid ;  idx < lwid ;  idx += 1)
		  result = less_with_cascade(l.get_bit(idx), St0, result);
      } else {
	    for (unsigned idx = lwid ;  idx < rwid ;  idx += 1)
		  result = less_with_cascade(St0, r.get_bit(idx), result);
      }

      return result;
}

/*
 * This is the <= operator that applies to signed operands.
 */
vpip_bit_t vvm_binop_le_s(const vvm_bitset_t&l, const vvm_bitset_t&r)
{
      vpip_bit_t l_pad = l.get_bit(l.get_width()-1);
      vpip_bit_t r_pad = r.get_bit(r.get_width()-1);

	/* If l>=0 and r>=0, return $unsigned(l) <= $unsigned(r) */
      if (B_IS0(l_pad) && B_IS0(r_pad))
	    return vvm_binop_le(l, r);

	/* If l < 0 and r < 0, return $unsigned(r) <= $unsigned(l) */
      if (B_IS1(l_pad) && B_IS1(r_pad))
	    return vvm_binop_le(r, l);

	/* If l >= 0 and r < 0, return false; */
      if (B_IS0(l_pad) && B_IS1(r_pad))
	    return St0;

	/* if l < 0 and r >= 0, return true; */
      if (B_IS1(l_pad) && B_IS0(r_pad))
	    return St1;

	/* Otherwise, one or the other side is unknown. Return X. */
      return StX;
}

vpip_bit_t vvm_binop_gt(const vvm_bitset_t&l, const vvm_bitset_t&r)
{
      vpip_bit_t result = St0;

      const unsigned lwid = l.get_width();
      const unsigned rwid = r.get_width();
      const unsigned common = (lwid < rwid)? lwid : rwid;

      for (unsigned idx = 0 ;  idx < common ;  idx += 1)
	    result = greater_with_cascade(l.get_bit(idx),
					  r.get_bit(idx),
					  result);

      if (lwid > rwid) {
	    for (unsigned idx = rwid ;  idx < lwid ;  idx += 1)
		  result = greater_with_cascade(l.get_bit(idx), St0, result);
      } else {
	    for (unsigned idx = lwid ;  idx < rwid ;  idx += 1)
		  result = greater_with_cascade(St0, r.get_bit(idx), result);
      }

      return result;
}

/*
 * This is the > operator that applies to signed operands.
 */
vpip_bit_t vvm_binop_gt_s(const vvm_bitset_t&l, const vvm_bitset_t&r)
{
      vpip_bit_t l_pad = l.get_bit(l.get_width()-1);
      vpip_bit_t r_pad = r.get_bit(r.get_width()-1);

	/* If l>=0 and r>=0, return $unsigned(l) > $unsigned(r) */
      if (B_IS0(l_pad) && B_IS0(r_pad))
	    return vvm_binop_gt(l, r);

	/* If l < 0 and r < 0, return $unsigned(r) > $unsigned(l) */
      if (B_IS1(l_pad) && B_IS1(r_pad))
	    return vvm_binop_gt(r, l);

	/* If l >= 0 and r < 0, return true; */
      if (B_IS0(l_pad) && B_IS1(r_pad))
	    return St1;

	/* if l < 0 and r >= 0, return false; */
      if (B_IS1(l_pad) && B_IS0(r_pad))
	    return St0;

	/* Otherwise, one or the other side is unknown. Return X. */
      return StX;
}

vpip_bit_t vvm_binop_ge(const vvm_bitset_t&l, const vvm_bitset_t&r)
{
      vpip_bit_t result = St1;

      const unsigned lwid = l.get_width();
      const unsigned rwid = r.get_width();
      const unsigned common = (lwid < rwid)? lwid : rwid;

      for (unsigned idx = 0 ;  idx < common ;  idx += 1)
	    result = greater_with_cascade(l.get_bit(idx),
					  r.get_bit(idx), result);

      if (lwid > rwid) {
	    for (unsigned idx = rwid ;  idx < lwid ;  idx += 1)
		  result = greater_with_cascade(l.get_bit(idx), St0, result);
      } else {
	    for (unsigned idx = lwid ;  idx < rwid ;  idx += 1)
		  result = greater_with_cascade(St0, r.get_bit(idx), result);
      }

      return result;
}

/*
 * This is the >= operator that applies to signed operands.
 */
vpip_bit_t vvm_binop_ge_s(const vvm_bitset_t&l, const vvm_bitset_t&r)
{
      vpip_bit_t l_pad = l.get_bit(l.get_width()-1);
      vpip_bit_t r_pad = r.get_bit(r.get_width()-1);

	/* If l>=0 and r>=0, return $unsigned(l) >= $unsigned(r) */
      if (B_IS0(l_pad) && B_IS0(r_pad))
	    return vvm_binop_ge(l, r);

	/* If l < 0 and r < 0, return $unsigned(r) >= $unsigned(l) */
      if (B_IS1(l_pad) && B_IS1(r_pad))
	    return vvm_binop_ge(r, l);

	/* If l >= 0 and r < 0, return true; */
      if (B_IS0(l_pad) && B_IS1(r_pad))
	    return St1;

	/* if l < 0 and r >= 0, return false; */
      if (B_IS1(l_pad) && B_IS0(r_pad))
	    return St0;

	/* Otherwise, one or the other side is unknown. Return X. */
      return StX;
}

vpip_bit_t vvm_binop_land(const vvm_bitset_t&l, const vvm_bitset_t&r)
{
      vpip_bit_t res1 = vvm_unop_or(l);
      vpip_bit_t res2 = vvm_unop_or(r);
      return B_AND(res1, res2);
}

vpip_bit_t vvm_binop_lor(const vvm_bitset_t&l, const vvm_bitset_t&r)
{
      vpip_bit_t res1 = vvm_unop_or(l);
      vpip_bit_t res2 = vvm_unop_or(r);
      return B_OR(res1, res2);
}

void vvm_ternary(vvm_bitset_t&v, vpip_bit_t c,
		 const vvm_bitset_t&t,
		 const vvm_bitset_t&f)
{
      if (B_IS0(c)) {
	    for (unsigned idx = 0 ;  idx < v.nbits ;  idx += 1)
		  v[idx] = (idx < f.nbits)? f[idx] : St0;
	    return;
      }
      if (B_IS1(c)) {
	    for (unsigned idx = 0 ;  idx < v.nbits ;  idx += 1)
		  v[idx] = (idx < t.nbits)? t[idx] : St0;
	    return;
      }

      for (unsigned idx = 0 ;  idx < v.nbits ;  idx += 1) {
	    vpip_bit_t tb = (idx < t.nbits)? t[idx] : St0;
	    vpip_bit_t fb = (idx < f.nbits)? f[idx] : St0;
	    if (B_EQ(tb, fb))
		  v[idx] = tb;
	    else
		  v[idx] = StX;
      }
}


/*
 * $Log: vvm_func.cc,v $
 * Revision 1.17  2001/07/25 03:10:50  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.16  2001/02/13 03:38:55  steve
 *  Binary or and nor resilient to bit widths.
 *
 * Revision 1.15  2001/02/10 01:57:18  steve
 *  Make the add func resilient to bit widths (PR#141)
 *
 * Revision 1.14  2001/02/07 21:47:13  steve
 *  Fix expression widths for rvalues and parameters (PR#131,132)
 *
 * Revision 1.13  2000/12/11 00:31:44  steve
 *  Add support for signed reg variables,
 *  simulate in t-vvm signed comparisons.
 *
 * Revision 1.12  2000/07/06 18:12:28  steve
 *  unop_not can take out width same as in width.
 *
 * Revision 1.11  2000/06/30 15:47:06  steve
 *  Reduce result is OK in ~ operator.
 *
 * Revision 1.10  2000/05/18 20:35:08  steve
 *  Ternary operator handles bit sizes.
 *
 * Revision 1.9  2000/05/18 20:23:40  steve
 *  Overcautious assert in shift is removed.
 *
 * Revision 1.8  2000/04/29 01:19:47  steve
 *  Proper bounds checking of the left operator of right shift.
 *
 * Revision 1.7  2000/04/26 03:32:40  steve
 *  AND handles argument padding if necessary.
 *
 * Revision 1.6  2000/03/26 16:55:41  steve
 *  Remove the vvm_bits_t abstract class.
 *
 * Revision 1.5  2000/03/26 16:28:31  steve
 *  vvm_bitset_t is no longer a template.
 *
 * Revision 1.4  2000/03/25 02:43:56  steve
 *  Remove all remain vvm_bitset_t return values,
 *  and disallow vvm_bitset_t copying.
 *
 * Revision 1.3  2000/03/24 02:43:37  steve
 *  vvm_unop and vvm_binop pass result by reference
 *  instead of returning a value.
 *
 * Revision 1.2  2000/03/22 04:26:41  steve
 *  Replace the vpip_bit_t with a typedef and
 *  define values for all the different bit
 *  values, including strengths.
 *
 * Revision 1.1  2000/03/13 00:02:34  steve
 *  Remove unneeded templates.
 *
 */

