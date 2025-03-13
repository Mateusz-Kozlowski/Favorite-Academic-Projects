package tests;

import expressions.Literal;
import instructions.Block;
import static org.junit.jupiter.api.Assertions.*;

class LiteralTest {

	@org.junit.jupiter.api.Test
	void test() {
		try {
			assertEquals(
				-5,
				Literal.of(-5).evaluate(new Block[] {})
			);

			Literal l = Literal.of(6);
			l.setValue(7);

			assertEquals(
				7,
				l.evaluate(new Block[] {})
			);
		}
		catch (Exception e) {
			System.out.println("Exception msg: " + e.getMessage());
			assert false;
		}
	}
}