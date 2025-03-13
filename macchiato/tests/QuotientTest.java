package tests;

import instructions.Block;
import expressions.Literal;
import expressions.Quotient;
import org.junit.jupiter.api.Assertions;

import static org.junit.jupiter.api.Assertions.*;

class QuotientTest {

	@org.junit.jupiter.api.Test
	void test() {
		try {
			Assertions.assertEquals(
				1,
				Quotient.of(Literal.of(6), Literal.of(6)).evaluate(new Block[] {})
			);
			Assertions.assertEquals(
				0,
				Quotient.of(Literal.of(0), Literal.of(6)).evaluate(new Block[] {})
			);
			Assertions.assertEquals(
				-1,
				Quotient.of(Literal.of(-6), Literal.of(6)).evaluate(new Block[] {})
			);
			Assertions.assertEquals(
				-2,
				Quotient.of(Literal.of(36), Literal.of(-18)).evaluate(new Block[] {})
			);
		}
		catch (Exception e) {
			System.out.println("Exception msg: " + e.getMessage());
			assert false;
		}

		try {
			Quotient.of(Literal.of(1), Literal.of(0)).evaluate(new Block[] {});
			assert false;
		}
		catch (Exception e) {
			assert true;
		}
	}
}