package tests;

import builders.BlockBuilder;
import expressions.Literal;
import instructions.Block;
import java.util.List;
import static org.junit.jupiter.api.Assertions.*;

class AssignmentTest {

	@org.junit.jupiter.api.Test
	void test() {
		try {
			Block block = new BlockBuilder()
				.declareVariable('a', Literal.of(1))
				.assign('a', Literal.of(2))
				.build();

			int res = block.startProgram(false);

			assertEquals(0, res);
			assertEquals(
				2,
				block.getVariableValue('a', new Block[] {block})
			);
		}
		catch (Exception e) {
			System.out.println("Exception msg: " + e.getMessage());
			assert false;
		}
	}
}