package tests;

import expressions.Literal;
import expressions.Sum;
import instructions.Block;

import static org.junit.jupiter.api.Assertions.*;

class SumTest {

	@org.junit.jupiter.api.Test
	void test() {
		try {
			assertEquals(
				7,
				Sum.of(Literal.of(1), Literal.of(6)).evaluate(new Block[] {})
			);
			assertEquals(
				6,
				Sum.of(Literal.of(0), Literal.of(6)).evaluate(new Block[] {})
			);
			assertEquals(
				12,
				Sum.of(Literal.of(6), Literal.of(6)).evaluate(new Block[] {})
			);
			assertEquals(
				0,
				Sum.of(Literal.of(0), Literal.of(0)).evaluate(new Block[] {})
			);
		}
		catch (Exception e) {
			System.out.println("Exception msg: " + e.getMessage());
			assert false;
		}
	}
}