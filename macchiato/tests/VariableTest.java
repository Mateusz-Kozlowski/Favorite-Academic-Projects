package tests;

import builders.BlockBuilder;
import expressions.Literal;
import expressions.Sum;
import expressions.Variable;
import instructions.Block;

import static org.junit.jupiter.api.Assertions.*;

class VariableTest {

	@org.junit.jupiter.api.Test
	void test() {
		try {
			Block block = new BlockBuilder()
				.declareVariable('a', Literal.of(5))
				.declareVariable('b', Literal.of(6))
				.assign('a', Sum.of(Variable.named('a'), Literal.of(1)))
				.assign('b', Sum.of(Variable.named('b'), Literal.of(2)))
				.build();

			int res = block.startProgram(false);

			assertEquals(res, 0);
			assertEquals(6, block.getVariableValue('a', new Block[] {block}));
			assertEquals(8, block.getVariableValue('b', new Block[] {block}));
		}
		catch (Exception e) {
			System.out.println("Exception msg: " + e.getMessage());
			assert false;
		}
	}
}