package tests;

import expressions.Diff;
import expressions.Literal;
import instructions.Block;

import static org.junit.jupiter.api.Assertions.*;

class DiffTest {
	@org.junit.jupiter.api.Test
	void test() {
		try {
			assertEquals(
				-5,
				Diff.of(Literal.of(1), Literal.of(6)).evaluate(new Block[] {})
			);
			assertEquals(
				-6,
				Diff.of(Literal.of(0), Literal.of(6)).evaluate(new Block[] {})
			);
			assertEquals(
				0,
				Diff.of(Literal.of(6), Literal.of(6)).evaluate(new Block[] {})
			);
			assertEquals(
				12,
				Diff.of(Literal.of(18), Literal.of(6)).evaluate(new Block[] {})
			);
			assertEquals(
				2,
				Diff.of(Literal.of(8), Literal.of(6)).evaluate(new Block[] {})
			);
		}
		catch (Exception e) {
			System.out.println("Exception msg: " + e.getMessage());
			assert false;
		}
	}
}